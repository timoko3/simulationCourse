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
    CLI::App app{"Measure one execution mode or compare all four on the same ELF program"};
    Options options;
    std::string mode;
    std::string program;

    configureCommandLine(app, options, mode, program);

    try {
        app.parse(argc, argv);
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
           << "\",\"cache\":\"cold\",\"warmups\":" << options.warmups
           << ",\"samples\":[";

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
    }

    throw std::invalid_argument("Invalid benchmark mode");
}

void configureCommandLine(CLI::App& app, Options& options,
                          std::string& mode, std::string& program)
{
    app.add_option("--mode", mode, "Execution mode")
        ->required()->check(CLI::IsMember({"naive", "threaded", "switch", "dbt", "comparison"}));

    app.add_option("--program", program, "Self-contained ELF program ending with exit code 0")
        ->required()->check(CLI::ExistingFile);

    app.add_option("--samples", options.samples, "Measured executions (four modes in comparison)")
        ->check(CLI::Range(minSamples, maxSamples));

    app.add_option("--warmups", options.warmups, "Unmeasured executions (four modes in comparison)")
        ->check(CLI::Range(minWarmups, maxWarmups));
}

void printSample(std::ostream& output, const Sample& sample)
{
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
