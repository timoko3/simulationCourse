#include <cstdlib>
#include <exception>
#include <iostream>

#include "generalFunctions/file.h"
#include "isa/definitions.hpp"
#include "toyFunctionalSim/functionalSimulator.hpp"

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <program.bin>\n";
        return EXIT_FAILURE;
    }

    try {
        const auto program = generalFunctions::readBinaryFile<simulator::Word>(argv[1]);

        if (program.empty()) {
            std::cerr << "Error: program is empty\n";
            return EXIT_FAILURE;
        }

        simulator::FunctionalSimulator sim;

        sim.loadProgram(program);
        sim.runSimulation();

        return EXIT_SUCCESS;
    }
    catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}