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