#include <iostream>

#include "functionalSimulator.hpp"

namespace simulator
{

void
FunctionalSimulator::loadProgram(const std::vector<Word>& program){
    if(!isProgLoaded()){
        memory_.writeProgram(program);
    }
    else {
        std::cerr << "program is already loaded\n";
        // TODO: exception 
    }
    cpu_.getState().pc = 0;
}

void
FunctionalSimulator::runSimulation(){
    if(isProgLoaded()){
        stepResult stepRes = SR_NORMAL;
        while (stepRes == SR_NORMAL){
            stepRes = cpu_.step(memory_);
        }
    }
    else{
        std::cerr << "program is not loaded\n";
        // TODO: exception 
    }
    
}

}
