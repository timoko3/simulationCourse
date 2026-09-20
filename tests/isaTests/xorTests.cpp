#include "isaTestFixture.hpp"

class IsaXorTest : public IsaTest {};

TEST_F(IsaXorTest, FetchesDecodesAndExecutes)
{
    state.setReg(1, 0b1100u);
    state.setReg(2, 0b1010u);

    const simulator::Word raw = (1u << 21) | (2u << 16) | (3u << 11)
        | 0b011100u;
    memory.writeProgram({raw});

    const auto rawInstr = pipeline.fetch_.instrFetch(memory, state.pc);
    const auto instr = pipeline.decode_.instrDecode(rawInstr);
    ASSERT_EQ(instr.instrKind, simulator::InstrKind::I_XOR);
    EXPECT_EQ(instr.src1, 1u);
    EXPECT_EQ(instr.src2, 2u);
    EXPECT_EQ(instr.dst, 3u);

    execute(instr);

    EXPECT_EQ(state.getReg(3), 0b0110u);
    EXPECT_EQ(state.getReg(1), 0b1100u);
    EXPECT_EQ(state.getReg(2), 0b1010u);
    EXPECT_EQ(state.pc, 4u);
}
