#include "threadedExecutor.hpp"

#include "cpu.hpp"
#include "memory.hpp"

#include <stdexcept>

namespace simulator {

struct ThreadedExecutor::Context {
    Cpu& cpu;
    Memory& memory;

    BasicBlock block{};
    // The block owns these instructions. Refresh the cursors after replacing it.
    const Instruction* instr = nullptr;
    const Instruction* nextInstruction = nullptr;
    const Instruction* blockEnd = nullptr;
};

const ThreadedExecutor::HandlerTable ThreadedExecutor::handlersTable_ = [] {
    HandlerTable table{};
    table[static_cast<std::size_t>(InstrKind::I_ADD)] =
        &ThreadedExecutor::handleAdd;
    table[static_cast<std::size_t>(InstrKind::I_ADDI)] =
        &ThreadedExecutor::handleAddi;
    table[static_cast<std::size_t>(InstrKind::I_BEQ)] =
        &ThreadedExecutor::handleBeq;
    table[static_cast<std::size_t>(InstrKind::I_BEXT)] =
        &ThreadedExecutor::handleBext;
    table[static_cast<std::size_t>(InstrKind::I_LI)] =
        &ThreadedExecutor::handleLi;
    table[static_cast<std::size_t>(InstrKind::I_XOR)] =
        &ThreadedExecutor::handleXor;
    table[static_cast<std::size_t>(InstrKind::I_CLS)] =
        &ThreadedExecutor::handleCls;
    table[static_cast<std::size_t>(InstrKind::I_USAT)] =
        &ThreadedExecutor::handleUsat;
    table[static_cast<std::size_t>(InstrKind::I_LDREG)] =
        &ThreadedExecutor::handleLdReg;
    table[static_cast<std::size_t>(InstrKind::I_LD)] =
        &ThreadedExecutor::handleLd;
    table[static_cast<std::size_t>(InstrKind::I_ST)] =
        &ThreadedExecutor::handleSt;
    table[static_cast<std::size_t>(InstrKind::I_LDP)] =
        &ThreadedExecutor::handleLdp;
    table[static_cast<std::size_t>(InstrKind::I_SBIT)] =
        &ThreadedExecutor::handleSbit;
    table[static_cast<std::size_t>(InstrKind::I_J)] =
        &ThreadedExecutor::handleJ;
    table[static_cast<std::size_t>(InstrKind::I_SYSCALL)] =
        &ThreadedExecutor::handleSyscall;
    return table;
}();

void ThreadedExecutor::run(Cpu& cpu, Memory& memory)
{
    Context ctx{cpu, memory};

    prepareNext(ctx)(&ctx);
}

void ThreadedExecutor::loadNextBlock(Context& ctx)
{
    ctx.block = ctx.cpu.getBasicBlock(ctx.memory, ctx.cpu.getState().pc);
    if (ctx.block.empty()) {
        throw std::out_of_range("Cannot execute an empty basic block");
    }
    ctx.nextInstruction = ctx.block.data();
    ctx.blockEnd = ctx.nextInstruction + ctx.block.size();
}

inline ThreadedExecutor::Handler ThreadedExecutor::prepareNext(Context& ctx)
{
    if (ctx.nextInstruction == ctx.blockEnd) {
        loadNextBlock(ctx);
    }
    ctx.instr = ctx.nextInstruction++;

    const auto index = static_cast<std::size_t>(ctx.instr->instrKind);
    const Handler handler = handlersTable_.at(index);
    if (handler == nullptr) {
        throw std::out_of_range("No threaded handler for instruction kind");
    }
    
    return handler;
}

#define DEFINE_HANDLER(Name)                                            \
    void ThreadedExecutor::handle##Name(Context* ctx)                   \
    {                                                                   \
        ctx->cpu.getSimplePipeline().exec_.exec##Name(                  \
            ctx->cpu.getState(), *ctx->instr);                          \
        const Handler next = prepareNext(*ctx);                         \
        [[clang::musttail]] return next(ctx);                           \
    }

DEFINE_HANDLER(Add)
DEFINE_HANDLER(Addi)
DEFINE_HANDLER(Beq)
DEFINE_HANDLER(Bext)
DEFINE_HANDLER(Li)
DEFINE_HANDLER(Xor)
DEFINE_HANDLER(Cls)
DEFINE_HANDLER(Usat)
DEFINE_HANDLER(Sbit)
DEFINE_HANDLER(J)

#undef DEFINE_HANDLER

#define DEFINE_MEMORY_HANDLER(Name)                                     \
    void ThreadedExecutor::handle##Name(Context* ctx)                   \
    {                                                                   \
        ctx->cpu.getSimplePipeline().exec_.exec##Name(                  \
            ctx->cpu.getState(), ctx->memory, *ctx->instr);             \
        const Handler next = prepareNext(*ctx);                         \
        [[clang::musttail]] return next(ctx);                           \
    }

DEFINE_MEMORY_HANDLER(LdReg)
DEFINE_MEMORY_HANDLER(Ld)
DEFINE_MEMORY_HANDLER(St)
DEFINE_MEMORY_HANDLER(Ldp)

#undef DEFINE_MEMORY_HANDLER

void ThreadedExecutor::handleSyscall(Context* ctx){
    ctx->cpu.getSimplePipeline().exec_.execSyscall(
        ctx->cpu.getState(), *ctx->instr);
    const Handler next = prepareNext(*ctx);
    [[clang::musttail]] return next(ctx);
}

}
