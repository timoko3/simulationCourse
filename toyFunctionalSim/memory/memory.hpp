#pragma once

#include <vector>
#include <cstdint>

#include "isa/definitions.hpp"
#include "mappedMemory.hpp"

namespace simulator{

constexpr std::size_t MEMORY_SIZE = 1024 * 1024;

struct Memory{
    Memory() : data_(MEMORY_SIZE) {}

    const Byte* data() const noexcept { return data_.data(); }
    std::size_t size() const noexcept { return data_.size(); }

    bool operator==(const Memory& other) const;
    bool operator!=(const Memory& other) const { return !(*this == other); }

    Word read32(Addr addr) const;
    void write32(Addr addr, Word word);
    void loadData(Addr addr, const Byte* source, std::size_t byteCount);
    void fillData(Addr addr, std::size_t byteCount, Byte value = 0);

    void writeProgram(const std::vector<Word>& program);

    std::size_t getPageSize() const;
    void protectCode(Addr addr, std::size_t byteCount);
    void unprotectCode(Addr addr, std::size_t byteCount);
    bool isWriteProtected(Addr addr, std::size_t byteCount) const {
        return data_.isWriteProtected(addr, byteCount);
    }

private:
    MappedMemory data_;
};

}
