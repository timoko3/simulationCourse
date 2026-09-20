#include "isaTestFixture.hpp"

class IsaLdRegTest : public IsaTest {};

TEST_F(IsaLdRegTest, FetchesDecodesAndExecutes)
{
    state.setReg(1, 4u);
    state.setReg(2, 4u);

    const simulator::Word raw = (0b110101u << 26) | (1u << 21)
        | (3u << 16) | (0b11u << 14) | 2u;
    memory.writeProgram({raw, 0u, 0x12345678u});

    const auto rawInstr = pipeline.fetch_.instrFetch(memory, state.pc);
    const auto instr = pipeline.decode_.instrDecode(rawInstr);
    ASSERT_EQ(instr.instrKind, simulator::InstrKind::I_LDREG);
    EXPECT_EQ(instr.src1, 1u);
    EXPECT_EQ(instr.src2, 2u);
    EXPECT_EQ(instr.dst, 3u);

    execute(instr);

    EXPECT_EQ(state.getReg(3), 0x12345678u);
    EXPECT_EQ(state.pc, 4u);
}
