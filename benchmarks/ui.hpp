#pragma once

#include "benchmark.hpp"

#include <iosfwd>
#include <optional>
#include <string_view>

namespace benchmark::ui {

struct InputResult {
    std::optional<Options> options;
    int exitCode = 0;
};

InputResult readOptions(int argc, char** argv);

void printResults(std::ostream& output, const Options& options,
                  const std::vector<Sample>& samples);

void printError(std::ostream& output, std::string_view message);

} // namespace benchmark::ui
