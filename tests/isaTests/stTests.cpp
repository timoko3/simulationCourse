#include "isaTestFixture.hpp"

class IsaStTest : public IsaTest {};

TEST_F(IsaStTest, FetchesDecodesAndExecutes)
{
    state.setReg(1, 12u);
    state.setReg(2, 0x12345678u);

    const simulator::Word raw = (0b000110u << 26) | (1u << 21) | (2u << 16) | 0x3FFCu;
    memory.writeProgram({raw, 0u, 0u});

    const auto rawInstr = pipeline.fetch_.instrFetch(memory, state.pc);
    const auto instr = pipeline.decode_.instrDecode(rawInstr);
    ASSERT_EQ(instr.instrKind, simulator::InstrKind::I_ST);
    EXPECT_EQ(instr.src1, 1u);
    EXPECT_EQ(instr.src2, 2u);
    EXPECT_EQ(instr.dst, 0xFFFFFFFCu);

    execute(instr);

    EXPECT_EQ(memory.read32(8u), 0x12345678u);
    EXPECT_EQ(state.pc, 4u);
}
