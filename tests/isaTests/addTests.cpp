#include "isaTestFixture.hpp"

class IsaAddTest : public IsaTest {
protected:
    void add(simulator::Word dst, simulator::Word src1, simulator::Word src2)
    {
        simulator::Instruction instruction{};
        instruction.instrKind = simulator::InstrKind::I_ADD;
        instruction.src1 = src1;
        instruction.src2 = src2;
        instruction.dst = dst;
        execute(instruction);
    }
};

TEST_F(IsaAddTest, AddsTwoRegisters)
{
    state.setReg(1, 7);
    state.setReg(2, 5);

    add(3, 1, 2);

    EXPECT_EQ(state.getReg(3), 12u);
    EXPECT_EQ(state.getReg(1), 7u);
    EXPECT_EQ(state.getReg(2), 5u);
    EXPECT_EQ(state.pc, 4u);
}

TEST_F(IsaAddTest, FetchesDecodesAndExecutes)
{
    state.setReg(1, 7);
    state.setReg(2, 5);

    const simulator::Word add = (1u << 21) | (2u << 16) | (3u << 11)
        | 0b010101u;
    memory.writeProgram({add});

    const auto rawInstr = pipeline.fetch_.instrFetch(memory, state.pc);
    const auto instr = pipeline.decode_.instrDecode(rawInstr);
    ASSERT_EQ(instr.instrKind, simulator::InstrKind::I_ADD);
    EXPECT_EQ(instr.src1, 1u);
    EXPECT_EQ(instr.src2, 2u);
    EXPECT_EQ(instr.dst, 3u);

    execute(instr);

    EXPECT_EQ(state.getReg(3), 12u);
    EXPECT_EQ(state.getReg(1), 7u);
    EXPECT_EQ(state.getReg(2), 5u);
    EXPECT_EQ(state.pc, 4u);
}

TEST_F(IsaAddTest, AddsZeroOperands)
{
    state.setReg(3, 99);

    add(3, 1, 2);

    EXPECT_EQ(state.getReg(3), 0u);
}

TEST_F(IsaAddTest, AddingZeroPreservesValue)
{
    state.setReg(1, 42);

    add(3, 1, 2);
    EXPECT_EQ(state.getReg(3), 42u);

    add(4, 2, 1);
    EXPECT_EQ(state.getReg(4), 42u);
}

// Registers are uint32_t: addition wraps modulo 2^32.
TEST_F(IsaAddTest, WrapsOnUnsignedOverflow)
{
    state.setReg(1, 0xFFFFFFFFu);
    state.setReg(2, 1);

    add(3, 1, 2);

    EXPECT_EQ(state.getReg(3), 0u);
}

TEST_F(IsaAddTest, AddsTwoMaximumValues)
{
    state.setReg(1, 0xFFFFFFFFu);
    state.setReg(2, 0xFFFFFFFFu);

    add(3, 1, 2);

    EXPECT_EQ(state.getReg(3), 0xFFFFFFFEu);
}

TEST_F(IsaAddTest, CrossesSignedBoundary)
{
    state.setReg(1, 0x7FFFFFFFu);
    state.setReg(2, 1);

    add(3, 1, 2);

    EXPECT_EQ(state.getReg(3), 0x80000000u);
}

TEST_F(IsaAddTest, DestinationCanMatchFirstSource)
{
    state.setReg(1, 7);
    state.setReg(2, 5);

    add(1, 1, 2);

    EXPECT_EQ(state.getReg(1), 12u);
    EXPECT_EQ(state.getReg(2), 5u);
}

TEST_F(IsaAddTest, DestinationCanMatchSecondSource)
{
    state.setReg(1, 7);
    state.setReg(2, 5);

    add(2, 1, 2);

    EXPECT_EQ(state.getReg(2), 12u);
    EXPECT_EQ(state.getReg(1), 7u);
}

TEST_F(IsaAddTest, SourcesCanMatch)
{
    state.setReg(1, 7);

    add(3, 1, 1);

    EXPECT_EQ(state.getReg(3), 14u);
    EXPECT_EQ(state.getReg(1), 7u);
}

TEST_F(IsaAddTest, AllRegistersCanMatch)
{
    state.setReg(1, 7);

    add(1, 1, 1);

    EXPECT_EQ(state.getReg(1), 14u);
}

TEST_F(IsaAddTest, ChangesOnlyDestinationAndProgramCounter)
{
    for (simulator::Word reg = 0; reg < simulator::kNumRegs; ++reg) {
        state.setReg(reg, 100u + reg);
    }
    state.pc = 100;
    memory.writeProgram({0x12345678u, 0xABCDEF01u});
    const auto originalMemory = memory.data;

    add(31, 1, 2);

    EXPECT_EQ(state.getReg(31), 203u);
    for (simulator::Word reg = 0; reg < 31; ++reg) {
        EXPECT_EQ(state.getReg(reg), 100u + reg) << "register " << reg;
    }
    EXPECT_EQ(state.pc, 104u);
    EXPECT_EQ(memory.data, originalMemory);
}

TEST_F(IsaAddTest, DecodesHighRegisterNumbers)
{
    const simulator::Word raw =
        (30u << 21) | (29u << 16) | (31u << 11) | 0b010101u;

    const auto instruction = pipeline.decode_.instrDecode(raw);

    ASSERT_EQ(instruction.instrKind, simulator::InstrKind::I_ADD);
    EXPECT_EQ(instruction.src1, 30u);
    EXPECT_EQ(instruction.src2, 29u);
    EXPECT_EQ(instruction.dst, 31u);
}
