#include "isaTestFixture.hpp"

class IsaBeqTest : public IsaTest {};

TEST_F(IsaBeqTest, FetchesDecodesAndExecutes)
{
    state.setReg(1, 7u);
    state.setReg(2, 7u);
    state.pc = 8u;

    const simulator::Word raw = (0b111111u << 26) | (1u << 21)
        | (2u << 16) | 0xFFFFu;
    memory.writeProgram({0u, 0u, raw});

    const auto rawInstr = pipeline.fetch_.instrFetch(memory, state.pc);
    const auto instr = pipeline.decode_.instrDecode(rawInstr);
    ASSERT_EQ(instr.instrKind, simulator::InstrKind::I_BEQ);
    EXPECT_EQ(instr.src1, 1u);
    EXPECT_EQ(instr.src2, 2u);
    EXPECT_EQ(instr.dst, 0xFFFFFFFFu);

    execute(instr);

    EXPECT_EQ(state.pc, 4u);
}
