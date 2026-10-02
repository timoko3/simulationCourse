#pragma once 

#include "loader.hpp"

#include <filesystem>

namespace simulator{

class ElfLoader : public Loader{
public:
    void loadProgram(const std::filesystem::path& filePath, Memory& memory) override;     
};

}