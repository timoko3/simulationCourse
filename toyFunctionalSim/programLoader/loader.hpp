#pragma once 

#include "memory.hpp"

#include <filesystem>

namespace simulator{

class Loader{
public: 
    virtual ~Loader() = default;

    virtual void loadProgram(const std::filesystem::path& filePath, Memory& memory) = 0; 
};

}