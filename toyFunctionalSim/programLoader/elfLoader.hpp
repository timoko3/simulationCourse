#pragma once 

#include "loader.hpp"

#include <filesystem>

namespace ELFIO {
class elfio;
class segment;
}

namespace simulator{

class ElfLoader : public Loader{

    void validateElf(const ELFIO::elfio& elf);
    void loadSegment(const ELFIO::segment& segment, Memory& memory);
public:
    void loadProgram(const std::filesystem::path& filePath, Memory& memory, CpuState& state) override;     
};

}
