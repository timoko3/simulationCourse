#include "syscallHandlers.hpp"

#include "syscall.hpp"

#include <iostream>

namespace simulator {

void SyscallHandlers::handle(CpuState& state, Word code) {
    const Word type = state.getReg(SYSCALL_NUM_REG);
    switch (type) {
        case SYSCALL_EXIT:
            throw ExitSyscall{type, code};
        default:
            std::cerr << "syscall with " << type << " not handled\n";
            return;
    }
}

} // namespace simulator
