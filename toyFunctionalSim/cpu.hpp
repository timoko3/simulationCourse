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

class Cpu{
    using iCache = cache::CacheLIRS<BasicBlock, Addr>; 

    CpuState state_;
    SimplePipeline simpPipeline_;
    iCache iCache_{I_CACHE_SIZE};

    BasicBlock createBasicBlock(Memory& memory, Addr startPc);
public:
    BasicBlock getBasicBlock(Memory& memory, Addr startPc);

    void stepBlock(Memory& memory);

    void run(Memory& memory);

    CpuState& getState(){ return state_; }
    const CpuState& getState() const { return state_; }

    SimplePipeline& getSimplePipeline() { return simpPipeline_; } 

    iCache& getICache() { return iCache_; } 
};

}
