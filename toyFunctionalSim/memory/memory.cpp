#include "memory.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace simulator{

void
Memory::writeProgram(const std::vector<Word>& program){
    if (program.size() > data_.size() / sizeof(Word)) {
        throw std::length_error("Program is too large");
    }

    const auto byteCount = program.size() * sizeof(Word);

    if (byteCount != 0) {
        std::memcpy(data_.data(), program.data(), byteCount);
    }
}

void Memory::write32(Addr addr, Word word)
{
    const auto offset = static_cast<std::size_t>(addr);
    if (offset > data_.size() || data_.size() - offset < sizeof(Word)) {
        throw std::out_of_range("Memory write out of bounds");
    }

    std::memcpy(data_.data() + offset, &word, sizeof(word));
}

void Memory::loadData(Addr addr,
                      const Byte* source,
                      std::size_t byteCount){
    const auto offset = static_cast<std::size_t>(addr);

    if (offset > data_.size() || byteCount > data_.size() - offset) {
        throw std::out_of_range("Memory data load out of bounds");
    }

    if (byteCount == 0) {
        return;
    }

    if (source == nullptr) {
        throw std::invalid_argument("Memory data source is null");
    }

    std::memmove(data_.data() + offset, source, byteCount);
}

void Memory::fillData(Addr addr,
                      std::size_t byteCount,
                      Byte value){
    const auto offset = static_cast<std::size_t>(addr);

    if (offset > data_.size() || byteCount > data_.size() - offset) {
        throw std::out_of_range("Memory data fill out of bounds");
    }

    if (byteCount != 0) {
        std::fill_n(data_.data() + offset, byteCount, value);
    }
}

Word Memory::read32(Addr addr) const
{
    const auto offset = static_cast<std::size_t>(addr);
    if (offset > data_.size() || data_.size() - offset < sizeof(Word)) {
        throw std::out_of_range("Memory read out of bounds");
    }

    Word word;
    std::memcpy(&word, data_.data() + offset, sizeof(word));
    return word;
}

std::size_t Memory::getPageSize() const
{
    return MappedMemory::pageSize();
}

void Memory::protectCode(Addr addr, std::size_t byteCount)
{
    data_.protect(addr, byteCount, MappedMemory::Access::ReadOnly);
}

void Memory::unprotectCode(Addr addr, std::size_t byteCount)
{
    data_.protect(addr, byteCount, MappedMemory::Access::ReadWrite);
}

bool Memory::operator==(const Memory& other) const
{
    return size() == other.size() && (size() == 0 || std::memcmp(data(), other.data(), size()) == 0);
}

}
