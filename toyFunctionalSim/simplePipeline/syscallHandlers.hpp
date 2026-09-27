#pragma once

#include "cpuState.hpp"

namespace simulator {

class SyscallHandlers {
public:
    void handle(CpuState& state, Word code);
};

} // namespace simulator
