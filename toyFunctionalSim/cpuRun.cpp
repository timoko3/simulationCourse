#include "cpu.hpp"
#include "dbtExecutor.hpp"

#include <stdexcept>

#if SIMCOURSE_HAS_MUSTTAIL
#include "threadedExecutor.hpp"
#endif

namespace simulator {

void Cpu::run(Memory& memory, DispatchMode dispatchMode, ExecutionMode execMode, std::size_t threshold)
{
    if(execMode == ExecutionMode::Standard){
        if(dispatchMode == DispatchMode::Naive){
            while (true) {
                stepInstruction(memory);
            }
        }
        else if(dispatchMode == DispatchMode::Switch){
            while (true) {
                interpretBlock(memory);
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
    else if(execMode == ExecutionMode::DBT){
        if(dispatchMode == DispatchMode::Switch){
            DbtExecutor dbtExecutor{DbtLib::LLVM};
            dbtExecutor.run(*this, memory, threshold);
        }
        else{
            throw std::logic_error("for DBT dispatch must be switch");
        }   
    }
    else{
        throw std::logic_error("invalid executionMode");
    }
}

}
