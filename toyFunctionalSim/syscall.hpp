#pragma once

#include <cstdint>

namespace simulator{

constexpr uint32_t SYSCALL_EXIT = 60;

struct SimSyscall{
    uint32_t type;
    uint32_t code; 
};

}
