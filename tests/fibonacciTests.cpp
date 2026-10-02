#include <gtest/gtest.h>

#include "functionalSimulator.hpp"

TEST(FibonacciTest, ComputesFifthNumber)
{
    simulator::FunctionalSimulator sim;
    ASSERT_NO_THROW(sim.loadProgram(FIBONACCI_ELF_PATH));
    ASSERT_NO_THROW(sim.runSimulation());

    EXPECT_EQ(sim.getState().getReg(3), 5u);
    EXPECT_EQ(sim.getState().pc, 0x11B0u + 19u * sizeof(simulator::Word));
}
