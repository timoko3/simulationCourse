#include "cpu.hpp"

namespace simulator{

void 
Cpu::loadProgram(const std::vector<Word>& program){
    state_.memory->data.insert(state_.memory->data.end(), program.begin(), program.end());
}

void
Cpu::loadInstr(const Word instr){
   state_.memory->data.push_back(instr);     
}

void
Cpu::runCpu(){

}

}
