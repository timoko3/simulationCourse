#pragma once

#include <cstdint>

namespace simulator{

struct SimSyscall{
    uint32_t type;
    uint32_t code; 
};

}