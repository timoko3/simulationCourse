#include "benchmark.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <numeric>
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

Sample runRound(const Options& options, int round);
Sample runComparison(const std::filesystem::path& program, int round);
Sample runThresholdSweep(const Options& options, int round);

Execution execute(const std::filesystem::path& program, DispatchMode mode,
                  ExecutionMode executionMode = ExecutionMode::Standard,
                  std::size_t threshold = DEFAULT_DBT_THRESHOLD);
double measureExecution(FunctionalSimulator& simulator);

void verifyEqual(const Execution& first, const Execution& second);

} // namespace

std::vector<Sample> run(const Options& options)
{
    validateOptions(options);

    std::vector<Sample> samples;
    samples.reserve(options.samples);

    for (int round = 0; round < options.warmups + options.samples; ++round) {
        const Sample sample = runRound(options, round);

        if (round >= options.warmups) {
            samples.push_back(sample);
        }
    }

    return samples;
}

namespace {

Sample runRound(const Options& options, int round)
{
    const auto& program = options.program;

    switch (options.mode) {
        case Mode::Naive:
            return {std::nullopt, std::nullopt, std::nullopt,
                    execute(program, DispatchMode::Naive).nanoseconds};

        case Mode::Switch:
            return {execute(program, DispatchMode::Switch).nanoseconds, std::nullopt, std::nullopt, std::nullopt};

        case Mode::Threaded:
            return {std::nullopt, execute(program, DispatchMode::Threaded).nanoseconds, std::nullopt, std::nullopt};

        case Mode::DBT:
            return {std::nullopt, std::nullopt,
                    execute(program, DispatchMode::Switch, ExecutionMode::DBT).nanoseconds, std::nullopt};

        case Mode::Comparison:
            return runComparison(program, round);

        case Mode::ThresholdSweep:
            return runThresholdSweep(options, round);
    }

    throw std::invalid_argument("Invalid benchmark mode");
}

Sample runComparison(const std::filesystem::path& program, int round)
{
    std::array<std::optional<Execution>, 4> executions;
    const std::array modes{DispatchMode::Naive, DispatchMode::Switch,
                          DispatchMode::Threaded, DispatchMode::Switch};

    for (std::size_t offset = 0; offset < executions.size(); ++offset) {
        const auto index = (static_cast<std::size_t>(round) + offset) % executions.size();
        const auto dispatchMode = modes[index];
        const auto executionMode = index == 3 ? ExecutionMode::DBT : ExecutionMode::Standard;

        executions[index] = execute(program, dispatchMode, executionMode);
    }

    verifyEqual(*executions[0], *executions[1]);
    verifyEqual(*executions[0], *executions[2]);
    verifyEqual(*executions[0], *executions[3]);

    return {executions[1]->nanoseconds, executions[2]->nanoseconds,
            executions[3]->nanoseconds, executions[0]->nanoseconds};
}

Sample runThresholdSweep(const Options& options, int round)
{
    std::vector<std::size_t> order(options.thresholds.size() + 1);
    std::iota(order.begin(), order.end(), 0);
    const auto rotation = static_cast<std::size_t>(round) % order.size();
    std::rotate(order.begin(), order.begin() + rotation, order.end());

    if ((static_cast<std::size_t>(round) / order.size()) % 2 != 0) {
        std::reverse(order.begin(), order.end());
    }

    std::optional<Execution> reference;
    Sample sample;

    for (const auto index : order) {
        auto execution = index == 0
            ? execute(options.program, DispatchMode::Naive)
            : execute(options.program, DispatchMode::Switch, ExecutionMode::DBT, options.thresholds[index - 1]);

        if (reference) {
            verifyEqual(*reference, execution);
        }

        if (index == 0) {
            sample.naiveNs = execution.nanoseconds;
        }
        else {
            sample.thresholdDbtNs.emplace(options.thresholds[index - 1], execution.nanoseconds);
        }

        if (!reference) {
            reference = std::move(execution);
        }
    }

    return sample;
}

Execution execute(const std::filesystem::path& program, DispatchMode mode,
                  ExecutionMode executionMode, std::size_t threshold)
{
    FunctionalSimulator simulator{mode, executionMode, threshold};
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
        throw std::runtime_error("Execution modes produced different CPU or memory states");
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

    if (options.mode == Mode::ThresholdSweep) {
        auto thresholds = options.thresholds;
        std::sort(thresholds.begin(), thresholds.end());

        if (thresholds.empty() || std::adjacent_find(thresholds.begin(), thresholds.end()) != thresholds.end()) {
            throw std::invalid_argument("Thresholds must be a nonempty list of distinct nonnegative integers");
        }
    }
}

} // namespace

} // namespace benchmark
