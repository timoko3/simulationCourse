#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

#include <CLI/CLI.hpp>

#include "toyFunctionalSim/simulatorSettings.hpp"
#include "toyFunctionalSim/functionalSimulator.hpp"

int main(int argc, char* argv[])
{
    CLI::App app{"Functional ISA simulator"};
    std::string programPath;
    bool threaded = false;
    bool isDBT    = false;
    app.add_option("program", programPath, "ELF program")->required();
    app.add_flag("--threaded", threaded, "Use threaded dispatch (requires clang::musttail)");
    app.add_flag("--dbt", isDBT, "Use DBT optimization");

    try {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError& error) {
        return app.exit(error);
    }

    try {
        const auto dispatchMode = threaded ? simulator::DispatchMode::Threaded : simulator::DispatchMode::Switch;
        const auto execMode     = isDBT    ? simulator::ExecutionMode::DBT     : simulator::ExecutionMode::Standard;
        simulator::FunctionalSimulator sim{dispatchMode, execMode};

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
