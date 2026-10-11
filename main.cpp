#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

#include <CLI/CLI.hpp>

#include "toyFunctionalSim/dispatchMode.hpp"
#include "toyFunctionalSim/functionalSimulator.hpp"

int main(int argc, char* argv[])
{
    CLI::App app{"Functional ISA simulator"};
    std::string programPath;
    bool threaded = false;
    app.add_option("program", programPath, "ELF program")->required();
    app.add_flag("--threaded", threaded, "Use threaded dispatch (requires clang::musttail)");

    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError& error) {
        return app.exit(error);
    }

    try {
        const auto mode = threaded ? simulator::DispatchMode::Threaded : simulator::DispatchMode::Switch;
        simulator::FunctionalSimulator sim{mode};

        sim.loadProgram(programPath);
        const auto exitCode = sim.runSimulation();
        std::cout << "Program exited with code " << exitCode << '\n';

        return EXIT_SUCCESS;
    }
    catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
