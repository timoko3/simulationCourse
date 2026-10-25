#include "functionalSimulator.hpp"

#include <stdexcept>

#include "programLoader/elfLoader.hpp"
#include "syscall.hpp"

namespace simulator {

FunctionalSimulator::FunctionalSimulator(DispatchMode dispatchMode, ExecutionMode execMode)
    : dispatchMode_(dispatchMode), execMode_(execMode), loader_(std::make_unique<ElfLoader>()){}

uint32_t FunctionalSimulator::runSimulation()
{
    if (!isProgLoaded()) {
        throw std::logic_error("Program is not loaded");
    }

    try {
        cpu_.run(memory_, dispatchMode_, execMode_);
    }
    catch (const ExitSyscall& syscall) {
        if (syscall.type != SYSCALL_EXIT) {
            throw std::logic_error("this syscall type is not handled");
        }

        return syscall.code;
    }

    throw std::logic_error("CPU returned without an exit syscall");
}

void FunctionalSimulator::loadProgram(const std::filesystem::path& filePath)
{
    if (isProgLoaded()) {
        throw std::logic_error("program is already loaded.");
    }

    loader_->loadProgram(filePath, memory_, cpu_.getState());
    isProgLoad = true;
}

} // namespace simulator
