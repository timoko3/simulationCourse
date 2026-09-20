#pragma once

#include <gtest/gtest.h>

#include "simplePipeline/simplePipeline.hpp"

class IsaTest : public ::testing::Test {
protected:
    simulator::CpuState state{};
    simulator::Memory memory{};
    simulator::SimplePipeline pipeline{};

    void execute(const simulator::Instruction& instruction)
    {
        pipeline.exec_.execInstr(state, memory, instruction);
    }
};
