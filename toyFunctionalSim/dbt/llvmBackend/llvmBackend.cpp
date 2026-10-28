#include "llvmBackend.hpp"
#include "instructionEmitters.hpp"
#include "runtimeEmitters.hpp"
#include "../runtime.hpp"

#include <stdexcept>
#include <string>
#include <utility>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/DeadStoreElimination.h>
#include <llvm/Transforms/Scalar/EarlyCSE.h>

namespace simulator{

namespace {

struct LlvmCodeResources {
    std::shared_ptr<llvm::orc::LLJIT> jit;
    llvm::orc::ResourceTrackerSP tracker;

    LlvmCodeResources(std::shared_ptr<llvm::orc::LLJIT> jit, llvm::orc::ResourceTrackerSP tracker)
        : jit(std::move(jit)), tracker(std::move(tracker)) {}

    ~LlvmCodeResources() {
        if (auto error = tracker->remove()) {
            llvm::logAllUnhandledErrors(std::move(error), llvm::errs(),
                                       "Failed to remove JIT code: ");
        }
    }
};

}

LlvmBackend::LlvmBackend()
{
    if (llvm::InitializeNativeTarget() || llvm::InitializeNativeTargetAsmPrinter()) {
        throw std::runtime_error("Failed to initialize the native LLVM target");
    }

    auto jit = llvm::orc::LLJITBuilder().create();
    if (!jit) {
        throw std::runtime_error("Failed to create LLVM JIT: " +
                                 llvm::toString(jit.takeError()));
    }
    jit_ = std::move(*jit);

    passBuilder_.registerModuleAnalyses(moduleAnalyses_);
    passBuilder_.registerFunctionAnalyses(functionAnalyses_);
    functionAnalyses_.registerPass([this] {
        return llvm::ModuleAnalysisManagerFunctionProxy(moduleAnalyses_);
    });
    moduleAnalyses_.registerPass([this] {
        return llvm::FunctionAnalysisManagerModuleProxy(functionAnalyses_);
    });

    passes_.addPass(llvm::InstCombinePass());
    passes_.addPass(llvm::EarlyCSEPass());
    passes_.addPass(llvm::DSEPass());

    llvm::orc::SymbolMap runtimeSymbols;
    runtimeSymbols[jit_->mangleAndIntern(llvmEmitters::READ_WORD_SYMBOL)] = {
        llvm::orc::ExecutorAddr::fromPtr(&dbtRuntime::readGuestWord),
        llvm::JITSymbolFlags::Exported | llvm::JITSymbolFlags::Callable
    };
    runtimeSymbols[jit_->mangleAndIntern(llvmEmitters::WRITE_WORD_SYMBOL)] = {
        llvm::orc::ExecutorAddr::fromPtr(&dbtRuntime::writeGuestWord),
        llvm::JITSymbolFlags::Exported | llvm::JITSymbolFlags::Callable
    };
    runtimeSymbols[jit_->mangleAndIntern(llvmEmitters::SYSCALL_SYMBOL)] = {
        llvm::orc::ExecutorAddr::fromPtr(&dbtRuntime::handleSyscall),
        llvm::JITSymbolFlags::Exported | llvm::JITSymbolFlags::Callable
    };

    if (auto error = jit_->getMainJITDylib().define(
            llvm::orc::absoluteSymbols(std::move(runtimeSymbols)))) {
        throw std::runtime_error("Failed to register JIT runtime functions: " + llvm::toString(std::move(error)));
    }

    emitters_.set(InstrKind::I_ADD,     &llvmEmitters::emitRegRegReg<llvmEmitters::AddOperation>);
    emitters_.set(InstrKind::I_BEQ,     &llvmEmitters::emitBeq);
    emitters_.set(InstrKind::I_ADDI,    &llvmEmitters::emitRegRegImm<llvmEmitters::AddOperation>);
    emitters_.set(InstrKind::I_LD,      &llvmEmitters::emitLoad<&llvmEmitters::emitRegImmAddress<>>);
    emitters_.set(InstrKind::I_BEXT,    &llvmEmitters::emitRegRegReg<llvmEmitters::BitExtractOperation>);
    emitters_.set(InstrKind::I_LI,      &llvmEmitters::emitRegImm<llvmEmitters::CopyOperation>);
    emitters_.set(InstrKind::I_J,       &llvmEmitters::emitJ);
    emitters_.set(InstrKind::I_ST,      &llvmEmitters::emitStore<&llvmEmitters::emitRegImmAddress<&Instruction::dst>>);
    emitters_.set(InstrKind::I_LDP,     &llvmEmitters::emitLoadPair<&llvmEmitters::emitRegImmAddress<>>);
    emitters_.set(InstrKind::I_CLS,     &llvmEmitters::emitRegReg<llvmEmitters::CountLeadingOnesOperation>);
    emitters_.set(InstrKind::I_SBIT,    &llvmEmitters::emitRegImm<llvmEmitters::SetBitOperation, &Instruction::src2>);
    emitters_.set(InstrKind::I_USAT,    &llvmEmitters::emitRegRegImm<llvmEmitters::UnsignedSaturateOperation>);
    emitters_.set(InstrKind::I_XOR,     &llvmEmitters::emitRegRegReg<llvmEmitters::XorOperation>);
    emitters_.set(InstrKind::I_LDREG,   &llvmEmitters::emitLoad<&llvmEmitters::emitRegRegAddress<>>);
    emitters_.set(InstrKind::I_SYSCALL, &llvmEmitters::emitSyscall);
}

LlvmBackend::~LlvmBackend() = default;


std::unique_ptr<HostBasicBlock> 
LlvmBackend::translate(Addr pc, const BasicBlock& block) {
    auto context = std::make_unique<llvm::LLVMContext>();
    auto module = std::make_unique<llvm::Module>("translationEnvironment", *context);

    module->setDataLayout(jit_->getDataLayout());
    module->setTargetTriple(jit_->getTargetTriple());

    llvm::IRBuilder<> builder(*context);

    auto* pointerType = builder.getPtrTy();
    auto* signature = llvm::FunctionType::get(builder.getVoidTy(), {pointerType, pointerType}, false);

    const std::string functionName = "block_" + std::to_string(pc);
    auto* function = llvm::Function::Create(signature, llvm::Function::ExternalLinkage, functionName, module.get());
    function->setUWTableKind(llvm::UWTableKind::Async);

    auto* entry = llvm::BasicBlock::Create(*context, "entry", function);
    builder.SetInsertPoint(entry);

    function->getArg(0)->setName("state");
    function->getArg(1)->setName("memory");
    EmitContext emitContext{builder, function->getArg(0), function->getArg(1), pc};

    for(const auto& insn : block){
        emitContext.nextPc = builder.CreateAdd(llvmEmitters::loadPc(emitContext), builder.getInt32(sizeof(Word)), "next_pc");
        emitters_.emit(emitContext, insn);
        llvmEmitters::storePc(emitContext, emitContext.nextPc);

        emitContext.guestPc += sizeof(Word);
    }
    builder.CreateRetVoid(); // TODO: jump next block

    passes_.run(*function, functionAnalyses_);

    functionAnalyses_.clear();
    moduleAnalyses_.clear();

    auto tracker = jit_->getMainJITDylib().createResourceTracker();
    auto resources = std::make_shared<LlvmCodeResources>(jit_, tracker);
    if (auto error = jit_->addIRModule(tracker, llvm::orc::ThreadSafeModule(std::move(module), std::move(context)))) {
        throw std::runtime_error("Failed to add LLVM module: " + llvm::toString(std::move(error)));
    }

    auto symbol = jit_->lookup(functionName);
    if (!symbol) {
        throw std::runtime_error("Failed to find JIT function: " + llvm::toString(symbol.takeError()));
    }

    auto entryPoint = symbol->toPtr<HostBasicBlock::EntryPoint>();
    return std::make_unique<HostBasicBlock>(pc, entryPoint, std::move(resources));
}

}
