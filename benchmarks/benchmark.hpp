#pragma once

#include <filesystem>
#include <cstddef>
#include <map>
#include <optional>
#include <vector>

namespace benchmark {

enum class Mode { Naive, Threaded, Switch, DBT, Comparison, ThresholdSweep };

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
    std::vector<std::size_t> thresholds{0, 1, 3, 10, 30, 100, 300, 1000};
};

struct Sample {
    std::optional<double> switchNs;
    std::optional<double> threadedNs;
    std::optional<double> dbtNs;
    std::optional<double> naiveNs;
    std::map<std::size_t, double> thresholdDbtNs{};
};

std::vector<Sample> run(const Options& options);

} // namespace benchmark
