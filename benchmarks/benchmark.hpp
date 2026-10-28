#pragma once

#include <filesystem>
#include <optional>
#include <vector>

namespace benchmark {

enum class Mode { Naive, Threaded, Switch, DBT, Comparison };

inline constexpr int defaultSamples = 9;
inline constexpr int minSamples = 1;
inline constexpr int maxSamples = 1000;

inline constexpr int defaultWarmups = 2;
inline constexpr int minWarmups = 0;
inline constexpr int maxWarmups = 100;

struct Options {
    Mode mode = Mode::Comparison;
    std::filesystem::path program;
    int samples = defaultSamples;
    int warmups = defaultWarmups;
};

struct Sample {
    std::optional<double> switchNs;
    std::optional<double> threadedNs;
    std::optional<double> dbtNs;
    std::optional<double> naiveNs;
};

std::vector<Sample> run(const Options& options);

} // namespace benchmark
