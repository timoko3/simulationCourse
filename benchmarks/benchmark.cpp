#include "benchmark.hpp"

#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <string>
#include <utility>

#include "toyFunctionalSim/cpu.hpp"
#include "toyFunctionalSim/programLoader/elfLoader.hpp"
#include "toyFunctionalSim/syscall.hpp"

namespace benchmark {
namespace {

using namespace simulator;

constexpr uint32_t successfulExitCode = 0;

struct Program {
    CpuState state;
    Memory memory;
};

struct Execution {
    CpuState state;
    Memory memory;
    double nanoseconds;
};

void validateOptions(const Options& options);
Program loadProgram(const std::filesystem::path& path);

Sample runRound(Mode mode, const Program& program, int round);
Sample runComparison(const Program& program, int round);

Execution execute(const Program& program, DispatchMode mode);
double measureExecution(Cpu& cpu, Memory& memory, DispatchMode mode);

void verifyEqual(const Execution& first, const Execution& second);

} // namespace

std::vector<Sample> run(const Options& options)
{
    validateOptions(options);
    const Program program = loadProgram(options.program);

    std::vector<Sample> samples;
    samples.reserve(options.samples);

    for (int round = 0; round < options.warmups + options.samples; ++round) {
        const Sample sample = runRound(options.mode, program, round);

        if (round >= options.warmups) {
            samples.push_back(sample);
        }
    }

    return samples;
}

namespace {

Sample runRound(Mode mode, const Program& program, int round)
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

Sample runComparison(const Program& program, int round)
{
    const bool switchFirst = round % 2 == 0;
    const auto firstMode = switchFirst ? DispatchMode::Switch : DispatchMode::Threaded;
    const auto secondMode = switchFirst ? DispatchMode::Threaded : DispatchMode::Switch;

    const Execution first = execute(program, firstMode);
    const Execution second = execute(program, secondMode);

    verifyEqual(first, second);

    return switchFirst ? Sample{first.nanoseconds, second.nanoseconds}
                       : Sample{second.nanoseconds, first.nanoseconds};
}

Execution execute(const Program& program, DispatchMode mode)
{
    Cpu cpu;
    cpu.getState() = program.state;
    Memory memory = program.memory;

    const double nanoseconds = measureExecution(cpu, memory, mode);

    return {cpu.getState(), std::move(memory), nanoseconds};
}

double measureExecution(Cpu& cpu, Memory& memory, DispatchMode mode)
{
    const auto start = std::chrono::steady_clock::now();

    try {
        cpu.run(memory, mode);
    } 
    catch (const ExitSyscall& exit) {
        const auto elapsed = std::chrono::steady_clock::now() - start;

        if (exit.type != SYSCALL_EXIT || exit.code != successfulExitCode) {
            throw std::runtime_error("Program must terminate with exit syscall code " +
                                     std::to_string(successfulExitCode));
        }

        return std::chrono::duration<double, std::nano>(elapsed).count();
    }

    throw std::runtime_error("CPU returned without an exit syscall");
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

    if (options.mode != Mode::Threaded && options.mode != Mode::Switch &&
        options.mode != Mode::Comparison) {
        throw std::invalid_argument("Invalid benchmark mode");
    }
}

Program loadProgram(const std::filesystem::path& path)
{
    Program program;
    ElfLoader loader;
    loader.loadProgram(path, program.memory, program.state);

    return program;
}

} // namespace

} // namespace benchmark
