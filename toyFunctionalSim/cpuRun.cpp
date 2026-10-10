#include "cpu.hpp"

#include <stdexcept>

#if SIMCOURSE_HAS_MUSTTAIL
#include "threadedExecutor.hpp"
#endif

namespace simulator {

void Cpu::run(Memory& memory, DispatchMode dispatchMode)
{
    if(dispatchMode == DispatchMode::Switch){
        while (true) {
            stepBlock(memory);
        }
    }
    else if(dispatchMode == DispatchMode::Threaded){
#if SIMCOURSE_HAS_MUSTTAIL
        ThreadedExecutor::run(*this, memory);
#else
        throw std::runtime_error("Threaded dispatch is unavailable: rebuild with a compiler supporting clang::musttail");
#endif
    }
    else{
        throw std::logic_error("invalid dispatch parameter");
    }
}

}
