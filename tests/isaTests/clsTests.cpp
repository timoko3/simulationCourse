#include "isaTestFixture.hpp"

class IsaClsTest : public IsaTest {};

TEST_F(IsaClsTest, FetchesDecodesAndExecutes)
{
    state.setReg(2, 0xF0000000u);

    const simulator::Word raw = (3u << 21) | (2u << 16) | 0b001011u;
    memory.writeProgram({raw});

    const auto rawInstr = pipeline.fetch_.instrFetch(memory, state.pc);
    const auto instr = pipeline.decode_.instrDecode(rawInstr);
    ASSERT_EQ(instr.instrKind, simulator::InstrKind::I_CLS);
    EXPECT_EQ(instr.src1, 2u);
    EXPECT_EQ(instr.dst, 3u);

    execute(instr);

    EXPECT_EQ(state.getReg(3), 4u);
    EXPECT_EQ(state.pc, 4u);
}
