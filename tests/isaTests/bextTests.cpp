#include "isaTestFixture.hpp"

class IsaBextTest : public IsaTest {};

TEST_F(IsaBextTest, FetchesDecodesAndExecutes)
{
    state.setReg(1, 0b11010110u);
    state.setReg(2, 0b10101010u);

    const simulator::Word raw = (3u << 21) | (1u << 16)
        | (2u << 11) | 0b111110u;
    memory.writeProgram({raw});

    const auto rawInstr = pipeline.fetch_.instrFetch(memory, state.pc);
    const auto instr = pipeline.decode_.instrDecode(rawInstr);
    ASSERT_EQ(instr.instrKind, simulator::InstrKind::I_BEXT);
    EXPECT_EQ(instr.src1, 1u);
    EXPECT_EQ(instr.src2, 2u);
    EXPECT_EQ(instr.dst, 3u);

    execute(instr);

    EXPECT_EQ(state.getReg(3), 0b1001u);
    EXPECT_EQ(state.pc, 4u);
}
