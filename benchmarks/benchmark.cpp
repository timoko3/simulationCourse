#include "benchmark.hpp"

#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <string>

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
        case Mode::Switch:
            return {execute(program, DispatchMode::Switch).nanoseconds, std::nullopt};

        case Mode::Threaded:
            return {std::nullopt, execute(program, DispatchMode::Threaded).nanoseconds};

        case Mode::Comparison:
            return runComparison(program, round);
    }

    throw std::invalid_argument("Invalid benchmark mode");
}

Sample runComparison(const std::filesystem::path& program, int round)
{
    const bool isSwitch = round % 2 == 0;
    const auto firstMode = isSwitch ? DispatchMode::Switch : DispatchMode::Threaded;
    const auto secondMode = isSwitch ? DispatchMode::Threaded : DispatchMode::Switch;

    const Execution first = execute(program, firstMode);
    const Execution second = execute(program, secondMode);

    verifyEqual(first, second);

    return isSwitch ? Sample{first.nanoseconds, second.nanoseconds}
                       : Sample{second.nanoseconds, first.nanoseconds};
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
