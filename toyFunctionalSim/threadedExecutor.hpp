#pragma once

#include <array>
#include <cstddef>

#include "isa/isa.hpp"

#include "simplePipeline/execStage.hpp"

namespace simulator {

class Cpu;
struct Memory;

class ThreadedExecutor {
public:

    static void run(Cpu& cpu, Memory& memory);

private:
    struct Context;
    using Handler = void (*)(Context*);

    using HandlerTable =
        std::array<Handler, static_cast<std::size_t>(InstrKind::I_COUNT)>;

    static const HandlerTable handlersTable_;

    static void prepareNext(Context& ctx);
    static void handleAdd(Context* ctx);
    static void handleAddi(Context* ctx);
    static void handleBeq(Context* ctx);
    static void handleBext(Context* ctx);
    static void handleLi(Context* ctx);
    static void handleXor(Context* ctx);
    static void handleCls(Context* ctx);
    static void handleUsat(Context* ctx);
    static void handleLdReg(Context* ctx);
    static void handleLd(Context* ctx);
    static void handleSt(Context* ctx);
    static void handleLdp(Context* ctx);
    static void handleSbit(Context* ctx);
    static void handleJ(Context* ctx);
    static void handleSyscall(Context* ctx);
};

}
