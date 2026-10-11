#pragma once

#include <cstdint>
#include <filesystem>

#include "isa/definitions.hpp"

namespace simulator {

class Simulator {
public:
    virtual uint32_t runSimulation() = 0;

    virtual void loadProgram(const std::filesystem::path& progFilePath) = 0;
};

}
