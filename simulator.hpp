#pragma once

#include <filesystem>

#include "isa/definitions.hpp"

namespace simulator{

class Simulator{
public:
    virtual void runSimulation() = 0;

    virtual void loadProgram(const std::filesystem::path& progFilePath) = 0;
};

}