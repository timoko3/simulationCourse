#include <cstdlib>
#include <exception>
#include <iostream>

#include "generalFunctions/file.h"
#include "isa/definitions.hpp"
#include "toyFunctionalSim/functionalSimulator.hpp"

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <program.elf>\n";
        return EXIT_FAILURE;
    }

    try {
        simulator::FunctionalSimulator sim;

        sim.loadProgram(argv[1]);
        sim.runSimulation();

        return EXIT_SUCCESS;
    }
    catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}