#include "isaTestFixture.hpp"

class IsaSbitTest : public IsaTest {};

TEST_F(IsaSbitTest, FetchesDecodesAndExecutes)
{
    state.setReg(1, 0xFFFFFFFFu);

    const simulator::Word raw = (0b101110u << 26) | (2u << 21) | (1u << 16) | (5u << 11);
    memory.writeProgram({raw});

    const auto rawInstr = pipeline.fetch_.instrFetch(memory, state.pc);
    const auto instr = pipeline.decode_.instrDecode(rawInstr);
    ASSERT_EQ(instr.instrKind, simulator::InstrKind::I_SBIT);
    EXPECT_EQ(instr.src1, 1u);
    EXPECT_EQ(instr.src2, 5u);
    EXPECT_EQ(instr.dst, 2u);

    execute(instr);

    EXPECT_EQ(state.getReg(2), 32u);
    EXPECT_EQ(state.pc, 4u);
}
