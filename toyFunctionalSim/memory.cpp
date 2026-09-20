#include "memory.hpp"

#include <cstring>
#include <stdexcept>

namespace simulator{

void
Memory::writeProgram(const std::vector<Word>& program){
    if (program.size() > data.size() / sizeof(Word)) {
        throw std::length_error("Program is too large");
    }

    const auto byteCount = program.size() * sizeof(Word);

    if (byteCount != 0) {
        std::memcpy(data.data(), program.data(), byteCount);
    }
}

void Memory::write32(Addr addr, Word word)
{
    const auto offset = static_cast<std::size_t>(addr);
    if (offset > data.size() || data.size() - offset < sizeof(Word)) {
        throw std::out_of_range("Memory write out of bounds");
    }

    std::memcpy(data.data() + offset, &word, sizeof(word));
}

Word Memory::read32(Addr addr) const
{
    const auto offset = static_cast<std::size_t>(addr);
    if (offset > data.size() || data.size() - offset < sizeof(Word)) {
        throw std::out_of_range("Memory read out of bounds");
    }

    Word word;
    std::memcpy(&word, data.data() + offset, sizeof(word));
    return word;
}

}
