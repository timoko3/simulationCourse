#include <gtest/gtest.h>

#include "dbt/llvmBackend/llvmBackend.hpp"
#include "simplePipeline/execStage.hpp"

TEST(LlvmBackendTest, LiMatchesInterpreter)
{
    struct Case {
        simulator::Word dst, immediate;
    };
    const Case cases[] = {
        {3, 42},
        {3, 0},
        {3, 1},
        {3, 0x8000u},
        {31, 0xFFFFu},
        {0, 125},
    };

    simulator::LlvmBackend backend;
    simulator::ExecStage interpreter;
    simulator::Memory memory;
    const auto originalMemory = memory;

    for (const auto& test : cases) {
        SCOPED_TRACE(test.dst);
        simulator::CpuState state;
        for (simulator::Word reg = 0; reg < simulator::kNumRegs; ++reg) {
            state.setReg(reg, 100u + reg);
        }
        state.pc = 0x1000;
        auto expected = state;

        simulator::Instruction instruction{};
        instruction.instrKind = simulator::InstrKind::I_LI;
        instruction.src1 = test.immediate;
        instruction.dst = test.dst;

        interpreter.execLi(expected, instruction);
        auto translated = backend.translate(state.pc, {instruction});
        translated->getEntryPoint()(&state, &memory, translated->getDescriptor());

        for (simulator::Word reg = 0; reg < simulator::kNumRegs; ++reg) {
            EXPECT_EQ(state.getReg(reg), expected.getReg(reg)) << "register " << reg;
        }
        EXPECT_EQ(state.pc, expected.pc);
        EXPECT_EQ(memory, originalMemory);
    }
}

TEST(LlvmBackendTest, LiBlockCanBeExecutedRepeatedly)
{
    simulator::LlvmBackend backend;
    simulator::Instruction first{};
    first.instrKind = simulator::InstrKind::I_LI;
    first.src1 = 7;
    first.dst = 3;
    auto second = first;
    second.src1 = 42;
    second.dst = 31;
    auto translated = backend.translate(0x2000, {first, second});

    simulator::CpuState state;
    simulator::Memory memory;
    for (simulator::Word value : {100u, 200u}) {
        state.pc = 0x2000;
        state.setReg(3, value);
        state.setReg(31, value);
        translated->getEntryPoint()(&state, &memory, translated->getDescriptor());
        EXPECT_EQ(state.getReg(3), 7u);
        EXPECT_EQ(state.getReg(31), 42u);
        EXPECT_EQ(state.pc, 0x2008u);
    }
}

TEST(LlvmBackendTest, GroupedEmittersMatchInterpreterInMixedBlock)
{
    const auto instruction = [](simulator::InstrKind kind, simulator::Word dst,
                                simulator::Word src1, simulator::Word src2 = 0) {
        simulator::Instruction result{};
        result.instrKind = kind;
        result.dst = dst;
        result.src1 = src1;
        result.src2 = src2;
        return result;
    };
    const simulator::BasicBlock block{
        instruction(simulator::InstrKind::I_ADDI, 1, 1, 1),
        instruction(simulator::InstrKind::I_LI, 2, 0xFFFFu),
        instruction(simulator::InstrKind::I_ADD, 3, 1, 2),
        instruction(simulator::InstrKind::I_XOR, 1, 2, 3),
        instruction(simulator::InstrKind::I_ADDI, 31, 1, 0xFFFFFFFFu),
    };

    simulator::LlvmBackend backend;
    auto translated = backend.translate(0xFFFFFFF0u, block);
    simulator::ExecStage interpreter;
    simulator::Memory memory;
    for (simulator::Word initial : {0u, 0xFFFFFFFFu}) {
        simulator::CpuState state;
        state.pc = 0xFFFFFFF0u;
        state.setReg(1, initial);
        auto expected = state;
        interpreter.execAddi(expected, block[0]);
        interpreter.execLi(expected, block[1]);
        interpreter.execAdd(expected, block[2]);
        interpreter.execXor(expected, block[3]);
        interpreter.execAddi(expected, block[4]);

        translated->getEntryPoint()(&state, &memory, translated->getDescriptor());
        for (simulator::Word reg = 0; reg < simulator::kNumRegs; ++reg) {
            EXPECT_EQ(state.getReg(reg), expected.getReg(reg)) << "register " << reg;
        }
        EXPECT_EQ(state.pc, expected.pc);
    }
}
