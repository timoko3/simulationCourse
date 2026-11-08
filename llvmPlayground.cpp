#include <memory>
#include <utility>
#include <cstdint>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Error.h>

int main(){
    if (llvm::InitializeNativeTarget() || llvm::InitializeNativeTargetAsmPrinter()) {
        return 1;
    }

    llvm::ExitOnError check("JIT error: ");
    auto jit = check(llvm::orc::LLJITBuilder().create());

    auto context = std::make_unique<llvm::LLVMContext>();
    auto module = std::make_unique<llvm::Module>("playground", *context);

    module->setDataLayout(jit->getDataLayout());
    module->setTargetTriple(jit->getTargetTriple().str());

    llvm::IRBuilder<> builder(*context);

    auto* signature = llvm::FunctionType::get(builder.getInt32Ty(), {}, false);

    auto* function = llvm::Function::Create(signature, llvm::Function::ExternalLinkage, "answer", module.get());

    auto* entry = llvm::BasicBlock::Create(*context, "entry", function);
        builder.SetInsertPoint(entry);
    
    builder.CreateRet(builder.getInt32(42));

    module->print(llvm::outs(), nullptr);

    check(jit->addIRModule(
    llvm::orc::ThreadSafeModule(
        std::move(module), std::move(context))));

    auto symbol = check(jit->lookup("answer"));

    using AnswerFunction = std::int32_t (*)();

    auto answer = symbol.toPtr<AnswerFunction>();
    llvm::outs() << "Result: " << answer() << '\n';
}