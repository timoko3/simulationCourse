#include "benchmark.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <stdexcept>
#include <string>
#include <utility>

#include "toyFunctionalSim/functionalSimulator.hpp"

namespace benchmark {
namespace {

using namespace simulator;

constexpr uint32_t successfulExitCode = 0;

struct Execution {
    CpuState state;
    Memory memory;
    double nanoseconds;
};

void validateOptions(const Options& options);

Sample runRound(Mode mode, const std::filesystem::path& program, int round);
Sample runComparison(const std::filesystem::path& program, int round);

Execution execute(const std::filesystem::path& program, DispatchMode mode);
double measureExecution(FunctionalSimulator& simulator);

void verifyEqual(const Execution& first, const Execution& second);

} // namespace

std::vector<Sample> run(const Options& options)
{
    validateOptions(options);

    std::vector<Sample> samples;
    samples.reserve(options.samples);

    for (int round = 0; round < options.warmups + options.samples; ++round) {
        const Sample sample = runRound(options.mode, options.program, round);

        if (round >= options.warmups) {
            samples.push_back(sample);
        }
    }

    return samples;
}

namespace {

Sample runRound(Mode mode, const std::filesystem::path& program, int round)
{
    switch (mode) {
        case Mode::Naive:
            return {std::nullopt, std::nullopt, execute(program, DispatchMode::Naive).nanoseconds};

        case Mode::Switch:
            return {execute(program, DispatchMode::Switch).nanoseconds, std::nullopt, std::nullopt};

        case Mode::Threaded:
            return {std::nullopt, execute(program, DispatchMode::Threaded).nanoseconds, std::nullopt};

        case Mode::Comparison:
            return runComparison(program, round);
    }

    throw std::invalid_argument("Invalid benchmark mode");
}

Sample runComparison(const std::filesystem::path& program, int round)
{
    const std::array modes{DispatchMode::Naive, DispatchMode::Switch, DispatchMode::Threaded};
    auto order = modes;

    // Cycle through all six orders to balance position and predecessor effects.
    const auto rotation = round % modes.size();
    std::rotate(order.begin(), order.begin() + rotation, order.end());
    if ((round / modes.size()) % 2 != 0) {
        std::reverse(order.begin(), order.end());
    }

    std::optional<Execution> reference;
    Sample sample;

    for (const auto mode : order) {
        Execution execution = execute(program, mode);

        if (reference) {
            verifyEqual(*reference, execution);
        }

        switch (mode) {
            case DispatchMode::Naive:
                sample.naiveNs = execution.nanoseconds;
                break;

            case DispatchMode::Switch:
                sample.switchNs = execution.nanoseconds;
                break;

            case DispatchMode::Threaded:
                sample.threadedNs = execution.nanoseconds;
                break;
        }

        if (!reference) {
            reference = std::move(execution);
        }
    }

    return sample;
}

Execution execute(const std::filesystem::path& program, DispatchMode mode)
{
    FunctionalSimulator simulator{mode};
    simulator.loadProgram(program);

    const double nanoseconds = measureExecution(simulator);

    return {simulator.getState(), simulator.getMemory(), nanoseconds};
}

double measureExecution(FunctionalSimulator& simulator)
{
    const auto start = std::chrono::steady_clock::now();
    const auto exitCode = simulator.runSimulation();
    const auto elapsed = std::chrono::steady_clock::now() - start;

    if (exitCode != successfulExitCode) {
        throw std::runtime_error("Program must terminate with exit syscall code " +
                                 std::to_string(successfulExitCode));
    }

    return std::chrono::duration<double, std::nano>(elapsed).count();
}

void verifyEqual(const Execution& first, const Execution& second)
{
    const bool registersEqual = std::equal(
        std::begin(first.state.gpr_regs), std::end(first.state.gpr_regs),
        std::begin(second.state.gpr_regs));

    if (first.state.pc != second.state.pc || !registersEqual ||
        first.memory.data != second.memory.data) {
        throw std::runtime_error("Dispatch modes produced different CPU or memory states");
    }
}

void validateOptions(const Options& options)
{
    if (options.samples < minSamples || options.samples > maxSamples ||
        options.warmups < minWarmups || options.warmups > maxWarmups) {
        throw std::invalid_argument(
            "Samples must be " + std::to_string(minSamples) + ".." + std::to_string(maxSamples) +
            " and warmups " + std::to_string(minWarmups) + ".." + std::to_string(maxWarmups));
    }
}

} // namespace

} // namespace benchmark
