#pragma once

namespace simulator{

enum class DispatchMode {
    Switch,
    Threaded
};

enum class ExecutionMode {
    Standard, 
    DBT
};

enum class DbtLib{
    LLVM,
    ASM_JIT
};

}