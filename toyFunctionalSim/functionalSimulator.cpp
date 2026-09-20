#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "functionalSimulator.hpp"

#include "syscall.hpp"


namespace simulator
{

void
FunctionalSimulator::loadProgram(const std::vector<Word>& program){
    if(!isProgLoaded()){
        memory_.writeProgram(program);
    }
    else {
        throw std::logic_error("program is already loaded.");
    }
    cpu_.getState().pc = 0;

    isProgLoad = true;
}

void
FunctionalSimulator::runSimulation(){
    if (!isProgLoaded()) {
        throw std::logic_error("Program is not loaded");
    }

    try {
        cpu_.run(memory_);
    }
    catch (const SimSyscall& syscall) {
        if(syscall.type == SYSCALL_EXIT){
            std::cout << "Program exited with code "
                << syscall.code << '\n';
        }
        else{
            std::cerr << "syscall with "
                << syscall.type
                << " not handled" << '\n';

            throw std::logic_error("this syscall type is not handled");
        }
    }
}

}
