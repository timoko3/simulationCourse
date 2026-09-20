#include <gtest/gtest.h>

#include "functionalSimulator.hpp"
#include "generalFunctions/file.h"

TEST(FibonacciTest, ComputesFifthNumber)
{
    const auto program = generalFunctions::readBinaryFile<simulator::Word>(
        FIBONACCI_BINARY_PATH);
    ASSERT_FALSE(program.empty());

    simulator::FunctionalSimulator sim;
    sim.loadProgram(program);
    ASSERT_NO_THROW(sim.runSimulation());

    EXPECT_EQ(sim.getState().getReg(3), 5u);
    EXPECT_EQ(sim.getState().pc, 19u * sizeof(simulator::Word));
}
