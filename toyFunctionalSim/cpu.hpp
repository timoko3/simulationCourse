#pragma once

#include <cstdint>
#include <vector>
#include <array>

#include "cpuState.hpp"
#include "memory.hpp"
#include "simplePipeline/simplePipeline.hpp"

#include "cacheResearch/cache.h"

namespace simulator{

constexpr size_t I_CACHE_SIZE = 64;

enum stepResult{
    SR_NORMAL,
    SR_SYSCALL
};

class Cpu{
    CpuState state_;
    SimplePipeline simpPipeline_;
    cache::CacheLIRS<Instruction, Word> iCache_{I_CACHE_SIZE};
public:
    stepResult step(Memory& memory);

    CpuState& getState(){ return state_; }
};

}