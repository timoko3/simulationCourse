#include "isaTestFixture.hpp"

class IsaLiTest : public IsaTest {
protected:
    void li(simulator::Word dst, simulator::Word src1)
    {
        simulator::Instruction instruction{};
        instruction.instrKind = simulator::InstrKind::I_LI;
        instruction.src1 = src1;
        instruction.dst = dst;
        execute(instruction);
    }
};

TEST_F(IsaLiTest, FetchesDecodesAndExecutes)
{
    state.setReg(1, 7);
    state.setReg(2, 5);

    const simulator::Word li = (0b011111 << 26) | (0b00000 << 21) | (2u << 16) | 125u;
    memory.writeProgram({li});

    const auto rawInstr = pipeline.fetch_.instrFetch(memory, state.pc);
    const auto instr = pipeline.decode_.instrDecode(rawInstr);
    ASSERT_EQ(instr.instrKind, simulator::InstrKind::I_LI);
    EXPECT_EQ(instr.src1, 125u);
    EXPECT_EQ(instr.dst, 2u);

    execute(instr);

    EXPECT_EQ(state.getReg(2), 125u);
    EXPECT_EQ(state.pc, 4u);
}