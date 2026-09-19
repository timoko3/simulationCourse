#pragma once

#include <cstdint>
#include <vector>
#include <array>

#include "cpuState.hpp"
#include "simplePipeline/simplePipeline.hpp"

namespace simulator{

enum stepResult{
    SR_NORMAL,
    SR_SYSCALL
};

class Cpu{
    CpuState state_;
    SimplePipeline simpPipeline_;
public:
    stepResult step();
    
    CpuState& getState(){ return state_; }
};

}