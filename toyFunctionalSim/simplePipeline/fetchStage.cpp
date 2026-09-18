#include "fetchStage.hpp"

namespace simulator{

Word
FetchStage::instrFetch(Cpu& cpu){
    CpuState state = cpu.getState(); 
    return state.memory->data[state.pc];
}

}