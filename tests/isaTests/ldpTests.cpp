#include "isaTestFixture.hpp"

class IsaLdpTest : public IsaTest {};

TEST_F(IsaLdpTest, FetchesDecodesAndExecutes)
{
    state.setReg(1, 12u);

    const simulator::Word raw = (0b111010u << 26) | (1u << 21) | (2u << 16) | (3u << 11) | 0x7FCu;
    memory.writeProgram({raw, 0u, 0x12345678u, 0xABCDEF01u});

    const auto rawInstr = pipeline.fetch_.instrFetch(memory, state.pc);
    const auto instr = pipeline.decode_.instrDecode(rawInstr);
    ASSERT_EQ(instr.instrKind, simulator::InstrKind::I_LDP);
    EXPECT_EQ(instr.src1, 1u);
    EXPECT_EQ(instr.src2, 0xFFFFFFFCu);
    EXPECT_EQ(instr.dst, 2u);
    EXPECT_EQ(instr.dst2, 3u);

    execute(instr);

    EXPECT_EQ(state.getReg(2), 0x12345678u);
    EXPECT_EQ(state.getReg(3), 0xABCDEF01u);
    EXPECT_EQ(state.pc, 4u);
}
