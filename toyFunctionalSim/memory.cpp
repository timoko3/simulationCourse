#include "memory.hpp"

#include <algorithm>
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

void Memory::loadData(Addr addr,
                      const Byte* source,
                      std::size_t byteCount){
    const auto offset = static_cast<std::size_t>(addr);

    if (offset > data.size() || byteCount > data.size() - offset) {
        throw std::out_of_range("Memory data load out of bounds");
    }

    if (byteCount == 0) {
        return;
    }

    if (source == nullptr) {
        throw std::invalid_argument("Memory data source is null");
    }

    std::memcpy(data.data() + offset, source, byteCount);
}

void Memory::fillData(Addr addr,
                      std::size_t byteCount,
                      Byte value){
    const auto offset = static_cast<std::size_t>(addr);

    if (offset > data.size() || byteCount > data.size() - offset) {
        throw std::out_of_range("Memory data fill out of bounds");
    }

    std::fill_n(data.begin() + offset, byteCount, value);
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
