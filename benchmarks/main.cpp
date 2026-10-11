#include "benchmark.hpp"
#include "ui.hpp"

#include <exception>
#include <iostream>

int main(int argc, char** argv)
{
    try {
        const auto input = benchmark::ui::readOptions(argc, argv);

        if (!input.options) {
            return input.exitCode;
        }

        const auto samples = benchmark::run(*input.options);

        benchmark::ui::printResults(std::cout, *input.options, samples);
    } catch (const std::exception& error) {
        benchmark::ui::printError(std::cerr, error.what());
        return 1;
    }
}
