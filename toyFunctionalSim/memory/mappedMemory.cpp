#include "mappedMemory.hpp"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <new>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <sys/mman.h>
#include <unistd.h>

namespace simulator {

MappedMemory::MappedMemory(std::size_t byteCount)
    : size_(byteCount), protectedPages_(byteCount / pageSize() + (byteCount % pageSize() != 0), 0)
{
    if (size_ == 0) {
        return;
    }

    auto* address = mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (address == MAP_FAILED) {
        throw std::bad_alloc();
    }

    data_ = static_cast<Byte*>(address);
}

MappedMemory::~MappedMemory()
{
    if (data_) {
        munmap(data_, size_);
    }
}

MappedMemory::MappedMemory(const MappedMemory& other) : MappedMemory(other.size_)
{
    if (size_ != 0) {
        std::memcpy(data_, other.data_, size_);
    }
}

MappedMemory::MappedMemory(MappedMemory&& other) noexcept
    : data_(std::exchange(other.data_, nullptr)), size_(std::exchange(other.size_, 0)),
      protectedPages_(std::move(other.protectedPages_)) {}

std::size_t MappedMemory::pageSize()
{
    static const auto size = [] {
        const auto result = sysconf(_SC_PAGESIZE);
        if (result <= 0) {
            throw std::system_error(errno, std::generic_category(), "Cannot obtain page size");
        }
        return static_cast<std::size_t>(result);
    }();

    return size;
}

void MappedMemory::protect(std::size_t offset, std::size_t byteCount, Access access)
{
    if (offset > size_ || byteCount > size_ - offset) {
        throw std::out_of_range("Memory protection range is out of bounds");
    }
    if (byteCount == 0) {
        return;
    }

    const auto pageStart = offset - offset % pageSize();
    const auto protection = access == Access::ReadOnly ? PROT_READ : PROT_READ | PROT_WRITE;
    if (mprotect(data_ + pageStart, offset + byteCount - pageStart, protection) != 0) {
        throw std::system_error(errno, std::generic_category(), "Cannot change page protection");
    }

    const auto first = pageStart / pageSize();
    const auto last = (offset + byteCount - 1) / pageSize();
    std::fill(protectedPages_.begin() + first, protectedPages_.begin() + last + 1,
              access == Access::ReadOnly);
}

bool MappedMemory::isWriteProtected(std::size_t offset, std::size_t byteCount) const
{
    if (offset > size_ || byteCount > size_ - offset) {
        throw std::out_of_range("Memory protection range is out of bounds");
    }
    if (byteCount == 0) {
        return false;
    }

    const auto first = offset / pageSize();
    const auto last = (offset + byteCount - 1) / pageSize();
    return std::any_of(protectedPages_.begin() + first, protectedPages_.begin() + last + 1,
                       [](Byte protectedPage) { return protectedPage != 0; });
}

}
