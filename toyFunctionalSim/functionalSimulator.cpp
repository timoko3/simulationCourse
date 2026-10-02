#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "functionalSimulator.hpp"

#include "syscall.hpp"

#include "programLoader/elfLoader.hpp"

namespace simulator
{

FunctionalSimulator::FunctionalSimulator() : loader_(std::make_unique<ElfLoader>()){}

void
FunctionalSimulator::loadProgram(const std::filesystem::path& filePath){
    if(!isProgLoaded()){
        loader_->loadProgram(filePath, memory_);
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
    catch (const ExitSyscall& syscall) {
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
