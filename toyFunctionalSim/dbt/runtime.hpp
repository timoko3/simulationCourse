#pragma once

#include <stdexcept>

#include "toyFunctionalSim/memory.hpp"
#include "toyFunctionalSim/simplePipeline/syscallHandlers.hpp"

namespace simulator::dbtRuntime {

inline void requireAligned(Addr address);

inline Word readGuestWord(Memory* memory, Addr address) {
    requireAligned(address);

    return memory->read32(address);
}

inline void writeGuestWord(Memory* memory, Addr address, Word value) {
    requireAligned(address);

    memory->write32(address, value);
}

inline void handleSyscall(CpuState* state, Word code) {
    SyscallHandlers handler;

    handler.handle(*state, code);
}

inline void requireAligned(Addr address) {
    if (address % sizeof(Word) != 0) {
        throw std::runtime_error("MisalignedAccess");
    }
}

}
