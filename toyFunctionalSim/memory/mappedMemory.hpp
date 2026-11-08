#pragma once

#include <cstddef>
#include <vector>

#include "isa/definitions.hpp"

namespace simulator {

class MappedMemory {
public:
    enum class Access { ReadOnly, ReadWrite };

    explicit MappedMemory(std::size_t byteCount);
    ~MappedMemory();

    MappedMemory(const MappedMemory& other);
    MappedMemory(MappedMemory&& other) noexcept;
    MappedMemory& operator=(const MappedMemory&) = delete;
    MappedMemory& operator=(MappedMemory&&) = delete;

    Byte* data() noexcept { return data_; }
    const Byte* data() const noexcept { return data_; }
    std::size_t size() const noexcept { return size_; }

    static std::size_t pageSize();
    void protect(std::size_t offset, std::size_t byteCount, Access access);
    bool isWriteProtected(std::size_t offset, std::size_t byteCount) const;

private:
    Byte* data_ = nullptr;
    std::size_t size_;
    std::vector<Byte> protectedPages_;
};

}
