#pragma once

#include "simulator.hpp"

#include "cpu.hpp"
#include "dispatchMode.hpp"
#include "memory.hpp"

#include "programLoader/loader.hpp"

#include <filesystem>
#include <memory>

namespace simulator {

class FunctionalSimulator : public Simulator {
    Cpu cpu_;
    Memory memory_;
    DispatchMode dispatchMode_;

    std::unique_ptr<Loader> loader_;
    bool isProgLoad = false;

public:
    explicit FunctionalSimulator(DispatchMode mode = DispatchMode::Switch);

    uint32_t runSimulation() override;

    void loadProgram(const std::filesystem::path& filePath) override;

    bool isProgLoaded() const { return isProgLoad; }

    const CpuState& getState() const { return cpu_.getState(); }
    const Memory& getMemory() const { return memory_; }
};

} // namespace simulator
