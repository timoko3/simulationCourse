#pragma once

#include <vector>
#include <cstdint>

#include "isa/definitions.hpp"

namespace simulator{

constexpr std::size_t MEMORY_SIZE = 1024 * 1024;

struct Memory{
    std::vector<uint8_t> data; 

    Memory() : data(MEMORY_SIZE, 0) {}

    Word read32(Addr addr) const;
    void write32(Addr addr, Word word);
    void loadData(Addr addr, const Byte* source, std::size_t byteCount);
    void fillData(Addr addr, std::size_t byteCount, Byte value = 0);

    void writeProgram(const std::vector<Word>& program);
};

}
