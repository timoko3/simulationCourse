#pragma once

#include <cstddef>
#include <cstdint>

namespace simulator{

using Word     = std::uint32_t;
using Addr     = std::uint32_t;
using Byte     = std::uint8_t;
using Register = std::uint32_t;

constexpr std::size_t SYSCALL_NUM_REG = 9; 

}
