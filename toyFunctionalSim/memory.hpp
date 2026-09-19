#pragma once

#include <vector>
#include <cstdint>

#include "isa/definitions.hpp"

namespace simulator{

struct Memory{
    std::vector<uint8_t> data; 

    Word read32(Addr addr) const;
    void write32(Addr addr, Word word);

    void writeProgram(const std::vector<Word>& program);
};

}
