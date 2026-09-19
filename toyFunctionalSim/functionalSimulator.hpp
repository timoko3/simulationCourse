#pragma once

#include "simulator.hpp"

#include "cpu.hpp"
#include "memory.hpp"

namespace simulator{

class FunctionalSimulator : public Simulator{
private:
    Cpu cpu_;
    Memory memory_;
    bool isProgLoad = 0;
public:
    void runSimulation() override;

    void loadProgram(const std::vector<Word>& program) override;

    bool isProgLoaded() const { return isProgLoad; }; 
};

}