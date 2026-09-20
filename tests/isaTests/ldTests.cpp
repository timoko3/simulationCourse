#include "isaTestFixture.hpp"

class IsaLdTest : public IsaTest {};

TEST_F(IsaLdTest, FetchesDecodesAndExecutes)
{
    state.setReg(1, 12u);

    const simulator::Word raw = (0b001010u << 26) | (1u << 21) | (2u << 16) | 0x3FFCu;
    memory.writeProgram({raw, 0u, 0x12345678u});

    const auto rawInstr = pipeline.fetch_.instrFetch(memory, state.pc);
    const auto instr = pipeline.decode_.instrDecode(rawInstr);
    ASSERT_EQ(instr.instrKind, simulator::InstrKind::I_LD);
    EXPECT_EQ(instr.src1, 1u);
    EXPECT_EQ(instr.src2, 0xFFFFFFFCu);
    EXPECT_EQ(instr.dst, 2u);

    execute(instr);

    EXPECT_EQ(state.getReg(2), 0x12345678u);
    EXPECT_EQ(state.pc, 4u);
}
