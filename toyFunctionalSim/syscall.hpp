#pragma once

#include <cstdint>

namespace simulator{

enum SyscallType : uint32_t {
    SYSCALL_EXIT = 60,
};

struct ExitSyscall{
    uint32_t type;
    uint32_t code; 
};

}
