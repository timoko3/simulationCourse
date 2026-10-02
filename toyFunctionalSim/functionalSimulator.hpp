#pragma once

#include "simulator.hpp"

#include "cpu.hpp"
#include "memory.hpp"

#include "programLoader/loader.hpp"

#include <filesystem>

namespace simulator{

class FunctionalSimulator : public Simulator{
private:
    Cpu cpu_;
    Memory memory_;

    std::unique_ptr<Loader> loader_;

    bool isProgLoad = 0;
public:
    FunctionalSimulator();

    void runSimulation() override;

    void loadProgram(const std::filesystem::path& filePath) override;

    bool isProgLoaded() const { return isProgLoad; }; 
    const CpuState& getState() const { return cpu_.getState(); }
};

}
