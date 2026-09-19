#pragma once

#include <vector>

#include "isa/definitions.hpp"

namespace simulator{

class Simulator{
public:
    virtual void runSimulation() = 0;

    virtual void loadProgram(const std::vector<Word>& program) = 0;
};

}