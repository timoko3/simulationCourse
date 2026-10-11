#include <gtest/gtest.h>

#include <stdexcept>

#include "functionalSimulator.hpp"

TEST(FibonacciTest, ComputesFifthNumber)
{
    simulator::FunctionalSimulator sim;
    ASSERT_NO_THROW(sim.loadProgram(FIBONACCI_ELF_PATH));
    ASSERT_EQ(sim.runSimulation(), 0u);

    EXPECT_EQ(sim.getState().getReg(3), 5u);
    EXPECT_EQ(sim.getState().pc, 0x11B0u + 19u * sizeof(simulator::Word));
}

TEST(FibonacciTest, ThreadedMode)
{
    simulator::FunctionalSimulator sim{simulator::DispatchMode::Threaded};
    ASSERT_NO_THROW(sim.loadProgram(FIBONACCI_ELF_PATH));
#if SIMCOURSE_HAS_MUSTTAIL
    ASSERT_EQ(sim.runSimulation(), 0u);
    EXPECT_EQ(sim.getState().getReg(3), 5u);
    EXPECT_EQ(sim.getState().pc, 0x11B0u + 19u * sizeof(simulator::Word));
#else
    const auto initialPc = sim.getState().pc;
    EXPECT_THROW(sim.runSimulation(), std::runtime_error);
    EXPECT_EQ(sim.getState().pc, initialPc);
#endif
}
