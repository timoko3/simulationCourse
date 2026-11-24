#pragma once

namespace simulator {

class Cpu;
class DbtExecutor;
struct Memory;
struct CodeWriteFault;

class SmcController {
public:
    SmcController(Cpu& cpu, Memory& memory, DbtExecutor* dbtExecutor = nullptr)
        : cpu_(cpu), memory_(memory), dbtExecutor_(dbtExecutor) {}

    void handleSMC(const CodeWriteFault& fault);

private:
    Cpu& cpu_;
    Memory& memory_;
    DbtExecutor* dbtExecutor_;
};

}
