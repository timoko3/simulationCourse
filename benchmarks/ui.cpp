#include "ui.hpp"

#include <iomanip>
#include <map>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>

#include <CLI/CLI.hpp>

namespace benchmark::ui {
namespace {

constexpr int resultPrecision = 6;

Mode getMode(const std::string& mode);
std::string_view modeName(Mode mode);

void configureCommandLine(CLI::App& app, Options& options,
                          std::string& mode, std::string& program);

void printSample(std::ostream& output, const Sample& sample);

} // namespace

InputResult readOptions(int argc, char** argv)
{
    CLI::App app{"Measure execution modes and DBT threshold sweeps on an ELF program"};
    Options options;
    std::string mode;
    std::string program;

    configureCommandLine(app, options, mode, program);

    try {
        app.parse(argc, argv);

        if (app.count("--thresholds") != 0 && mode != "threshold-sweep") {
            throw CLI::ValidationError("--thresholds", "requires --mode threshold-sweep");
        }
    } 
    catch (const CLI::ParseError& error) {
        return {std::nullopt, app.exit(error)};
    }

    options.program = program;
    options.mode = getMode(mode);

    return {std::move(options), 0};
}

void printResults(std::ostream& output, const Options& options,
                  const std::vector<Sample>& samples)
{
    output << std::setprecision(resultPrecision)
           << "{\"mode\":\"" << modeName(options.mode)
           << "\",\"cache\":\"cold\",\"warmups\":" << options.warmups;

    if (options.mode == Mode::ThresholdSweep) {
        output << ",\"thresholds\":[";

        for (std::size_t index = 0; index < options.thresholds.size(); ++index) {
            if (index != 0) {
                output << ',';
            }

            output << options.thresholds[index];
        }

        output << ']';
    }

    output << ",\"samples\":[";

    bool first = true;

    for (const auto& sample : samples) {
        if (!first) {
            output << ',';
        }

        printSample(output, sample);
        first = false;
    }

    output << "]}\n";
}

void printError(std::ostream& output, std::string_view message)
{
    output << "Benchmark failed: " << message << '\n';
}

namespace {

Mode getMode(const std::string& mode)
{
    const std::map<std::string, Mode> modes{
        {"naive", Mode::Naive},
        {"threaded", Mode::Threaded},
        {"switch", Mode::Switch},
        {"dbt", Mode::DBT},
        {"comparison", Mode::Comparison},
        {"threshold-sweep", Mode::ThresholdSweep},
    };

    return modes.at(mode);
}

std::string_view modeName(Mode mode)
{
    switch (mode) {
    case Mode::Naive:
        return "naive";

    case Mode::Threaded:
        return "threaded";

    case Mode::Switch:
        return "switch";

    case Mode::DBT:
        return "dbt";

    case Mode::Comparison:
        return "comparison";

    case Mode::ThresholdSweep:
        return "threshold-sweep";
    }

    throw std::invalid_argument("Invalid benchmark mode");
}

void configureCommandLine(CLI::App& app, Options& options,
                          std::string& mode, std::string& program)
{
    app.add_option("--mode", mode, "Execution mode")
        ->required()->check(CLI::IsMember({"naive", "threaded", "switch", "dbt", "comparison", "threshold-sweep"}));

    app.add_option("--program", program, "Self-contained ELF program ending with exit code 0")
        ->required()->check(CLI::ExistingFile);

    app.add_option("--samples", options.samples, "Measured rounds")
        ->check(CLI::Range(minSamples, maxSamples));

    app.add_option("--warmups", options.warmups, "Unmeasured warmup rounds")
        ->check(CLI::Range(minWarmups, maxWarmups));

    app.add_option("--thresholds", options.thresholds, "DBT thresholds for threshold-sweep, separated by commas")
        ->delimiter(',')->check(CLI::NonNegativeNumber)->capture_default_str();
}

void printSample(std::ostream& output, const Sample& sample)
{
    if (!sample.thresholdDbtNs.empty()) {
        output << "{\"naive_ns\":" << *sample.naiveNs << ",\"dbt_ns\":{";
        bool first = true;

        for (const auto& [threshold, nanoseconds] : sample.thresholdDbtNs) {
            if (!first) {
                output << ',';
            }

            output << '"' << threshold << "\":" << nanoseconds;
            first = false;
        }

        output << "}}";
        return;
    }

    output << '{';

    if (sample.naiveNs) {
        output << "\"naive_ns\":" << *sample.naiveNs;

        if (sample.switchNs || sample.threadedNs || sample.dbtNs) {
            output << ',';
        }
    }

    if (sample.switchNs) {
        output << "\"switch_ns\":" << *sample.switchNs;
    }

    if (sample.switchNs && sample.threadedNs) {
        output << ',';
    }

    if (sample.threadedNs) {
        output << "\"threaded_ns\":" << *sample.threadedNs;
    }

    if (sample.dbtNs) {
        if (sample.switchNs || sample.threadedNs) {
            output << ',';
        }

        output << "\"dbt_ns\":" << *sample.dbtNs;
    }

    output << '}';
}

} // namespace

} // namespace benchmark::ui
