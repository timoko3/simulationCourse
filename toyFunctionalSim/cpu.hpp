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
    using iCache = cache::CacheLIRS<Instruction, Word>; 

    CpuState state_;
    SimplePipeline simpPipeline_;
    iCache iCache_{I_CACHE_SIZE};
public:
    stepResult step(Memory& memory);

    void run(Memory& memory);

    CpuState& getState(){ return state_; }
    const CpuState& getState() const { return state_; }

    SimplePipeline& getSimplePipeline() { return simpPipeline_; } 

    iCache& getICache() { return iCache_; } 
};

}
