#include "isaTestFixture.hpp"
#include "syscall.hpp"

class IsaSyscallTest : public IsaTest {};

TEST_F(IsaSyscallTest, PassesTypeAndCodeToHandler)
{
    constexpr simulator::Word syscallType = 60;
    state.setReg(simulator::SYSCALL_NUM_REG, syscallType);

    simulator::Instruction instruction{};
    instruction.instrKind = simulator::InstrKind::I_SYSCALL;
    instruction.src1 = 42;

    try {
        execute(instruction);
        FAIL() << "Expected SimSyscall";
    }
    catch (const simulator::SimSyscall& syscall) {
        EXPECT_EQ(syscall.type, syscallType);
        EXPECT_EQ(syscall.code, 42u);
    }
}

TEST_F(IsaSyscallTest, FetchesDecodesAndExecutes)
{
    constexpr simulator::Word syscallType = 60;
    state.setReg(simulator::SYSCALL_NUM_REG, syscallType);

    const simulator::Word li = (0b000000 << 26) | (5u << 6) | 0b010100;
    memory.writeProgram({li});

    const auto rawInstr = pipeline.fetch_.instrFetch(memory, state.pc);
    const auto instr = pipeline.decode_.instrDecode(rawInstr);
    ASSERT_EQ(instr.instrKind, simulator::InstrKind::I_SYSCALL);
    EXPECT_EQ(instr.src1, 5u);

    try {
        execute(instr);
        FAIL() << "Expected SimSyscall";
    }
    catch (const simulator::SimSyscall& syscall) {
        EXPECT_EQ(syscall.type, syscallType);
        EXPECT_EQ(syscall.code, 5u);
    }
}