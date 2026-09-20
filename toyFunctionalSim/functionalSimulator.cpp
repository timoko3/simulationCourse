#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "functionalSimulator.hpp"

#include "syscall.hpp"

#include <sys/syscall.h>

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

    isProgLoad = true;
}

void
FunctionalSimulator::runSimulation(){
    if (!isProgLoaded()) {
        throw std::logic_error("Program is not loaded");
    }

    try {
        while (cpu_.step(memory_) == SR_NORMAL) {
        }
    }
    catch (const SimSyscall& syscall) {
        if(syscall.type == SYS_exit){
            std::cout << "Program exited with code "
                << syscall.code << '\n';
            std::exit(syscall.code);
        }
        else{
            std::cout << "syscall with "
                << syscall.type
                << " not handled" << '\n';
        }
    }
}

}
