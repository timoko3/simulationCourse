#include "isaTestFixture.hpp"

class IsaJTest : public IsaTest {};

TEST_F(IsaJTest, FetchesDecodesAndExecutes)
{
    constexpr simulator::Word index = 0x02000005u;
    const simulator::Word raw = (0b000011u << 26) | index;
    memory.writeProgram({raw});

    const auto rawInstr = pipeline.fetch_.instrFetch(memory, state.pc);
    const auto instr = pipeline.decode_.instrDecode(rawInstr);
    ASSERT_EQ(instr.instrKind, simulator::InstrKind::I_J);
    EXPECT_EQ(instr.src1, index);

    execute(instr);

    EXPECT_EQ(state.pc, 0x08000014u);
}
