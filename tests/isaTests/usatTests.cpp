#include "isaTestFixture.hpp"

class IsaUsatTest : public IsaTest {};

TEST_F(IsaUsatTest, FetchesDecodesAndExecutes)
{
    state.setReg(2, 300u);

    const simulator::Word raw = (0b010110u << 26) | (3u << 21)
        | (2u << 16) | (8u << 11);
    memory.writeProgram({raw});

    const auto rawInstr = pipeline.fetch_.instrFetch(memory, state.pc);
    const auto instr = pipeline.decode_.instrDecode(rawInstr);
    ASSERT_EQ(instr.instrKind, simulator::InstrKind::I_USAT);
    EXPECT_EQ(instr.src1, 2u);
    EXPECT_EQ(instr.src2, 8u);
    EXPECT_EQ(instr.dst, 3u);

    execute(instr);

    EXPECT_EQ(state.getReg(3), 255u);
    EXPECT_EQ(state.pc, 4u);
}
