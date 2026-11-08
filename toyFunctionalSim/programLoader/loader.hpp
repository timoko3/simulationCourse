#pragma once 

#include "memory/memory.hpp"
#include "cpuState.hpp"

#include <filesystem>

namespace simulator{

class Loader{
public: 
    virtual ~Loader() = default;

    virtual void loadProgram(const std::filesystem::path& filePath, Memory& memory, CpuState& state) = 0; 
};

}