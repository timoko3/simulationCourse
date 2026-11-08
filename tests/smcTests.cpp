#include <gtest/gtest.h>

#include <vector>

#include "cpu.hpp"
#include "syscall.hpp"

namespace {

using namespace simulator;

struct RunMode {
    DispatchMode dispatch;
    ExecutionMode execution;
    std::size_t threshold;
};

std::vector<RunMode> runModes();
Word loadImmediate(Word reg, Word value);
Word addImmediate(Word reg, Word value);
Word jump(Addr address);
Word store(Word valueReg, Word baseReg);
void runToExit(Cpu& cpu, Memory& memory, RunMode mode);

TEST(SmcTest, ModifiesCurrentBlock)
{
    for (const auto mode : runModes()) {
        SCOPED_TRACE(::testing::Message() << static_cast<int>(mode.dispatch) << ':'
                     << static_cast<int>(mode.execution) << ':' << mode.threshold);
        Cpu cpu;
        Memory memory;
        const Word replacement = loadImmediate(3, 77);

        memory.writeProgram({addImmediate(4, 1), store(2, 1), loadImmediate(3, 1),
                             loadImmediate(9, SYSCALL_EXIT), 0b010100u});
        cpu.getState().setReg(1, 8);
        cpu.getState().setReg(2, replacement);

        runToExit(cpu, memory, mode);

        EXPECT_EQ(cpu.getState().getReg(3), 77u);
        EXPECT_EQ(cpu.getState().getReg(4), 1u);
        EXPECT_EQ(memory.read32(8), replacement);
        EXPECT_EQ(cpu.getState().pc, 16u);
        EXPECT_TRUE(memory.isWriteProtected(8, sizeof(Word)));
    }
}

TEST(SmcTest, ModifiesPreviouslyExecutedBlock)
{
    for (const auto mode : runModes()) {
        SCOPED_TRACE(::testing::Message() << static_cast<int>(mode.dispatch) << ':'
                     << static_cast<int>(mode.execution) << ':' << mode.threshold);
        Cpu cpu;
        Memory memory;
        const auto writerPc = static_cast<Addr>(memory.getPageSize());
        const Word replacement = addImmediate(3, 2);
        const Word branchToExit = (0b111111u << 26) | (3u << 21) | (4u << 16) | 3u;

        memory.writeProgram({addImmediate(3, 1), branchToExit, jump(writerPc), 0,
                             loadImmediate(9, SYSCALL_EXIT), 0b010100u});
        memory.write32(writerPc, store(2, 1));
        memory.write32(writerPc + sizeof(Word), jump(0));
        cpu.getState().setReg(1, 0);
        cpu.getState().setReg(2, replacement);
        cpu.getState().setReg(4, 3);

        runToExit(cpu, memory, mode);

        EXPECT_EQ(cpu.getState().getReg(3), 3u);
        EXPECT_EQ(memory.read32(0), replacement);
        EXPECT_EQ(cpu.getState().pc, 20u);
        EXPECT_TRUE(memory.isWriteProtected(0, sizeof(Word)));
    }
}

std::vector<RunMode> runModes()
{
    std::vector<RunMode> modes{{DispatchMode::Naive, ExecutionMode::Standard, 0},
                               {DispatchMode::Switch, ExecutionMode::Standard, 0}};
#if SIMCOURSE_HAS_MUSTTAIL
    modes.push_back({DispatchMode::Threaded, ExecutionMode::Standard, 0});
#endif
#if SIMCOURSE_HAS_LLVM
    modes.push_back({DispatchMode::Switch, ExecutionMode::DBT, 0});
    modes.push_back({DispatchMode::Switch, ExecutionMode::DBT, 2});
#endif
    return modes;
}

Word loadImmediate(Word reg, Word value)
{
    return (0b011111u << 26) | (reg << 16) | value;
}

Word addImmediate(Word reg, Word value)
{
    return (0b110000u << 26) | (reg << 21) | (reg << 16) | value;
}

Word jump(Addr address)
{
    return (0b000011u << 26) | (address / sizeof(Word));
}

Word store(Word valueReg, Word baseReg)
{
    return (0b000110u << 26) | (baseReg << 21) | (valueReg << 16);
}

void runToExit(Cpu& cpu, Memory& memory, RunMode mode)
{
    try {
        cpu.run(memory, mode.dispatch, mode.execution, mode.threshold);
        FAIL() << "CPU returned without an exit syscall";
    }
    catch (const ExitSyscall& exit) {
        EXPECT_EQ(exit.type, SYSCALL_EXIT);
        EXPECT_EQ(exit.code, 0u);
    }
}

}
