#include "isaTestFixture.hpp"

class IsaAddiTest : public IsaTest {};

TEST_F(IsaAddiTest, FetchesDecodesAndExecutes)
{
    state.setReg(1, 12u);

    const simulator::Word raw = (0b110000u << 26) | (1u << 21)
        | (2u << 16) | 0xFFFBu;
    memory.writeProgram({raw});

    const auto rawInstr = pipeline.fetch_.instrFetch(memory, state.pc);
    const auto instr = pipeline.decode_.instrDecode(rawInstr);
    ASSERT_EQ(instr.instrKind, simulator::InstrKind::I_ADDI);
    EXPECT_EQ(instr.src1, 1u);
    EXPECT_EQ(instr.src2, 0xFFFFFFFBu);
    EXPECT_EQ(instr.dst, 2u);

    execute(instr);

    EXPECT_EQ(state.getReg(2), 7u);
    EXPECT_EQ(state.pc, 4u);
}
