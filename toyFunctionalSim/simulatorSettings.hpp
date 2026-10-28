#pragma once

#include <cstddef>

namespace simulator{

inline constexpr std::size_t DEFAULT_DBT_THRESHOLD = 1;

enum class DispatchMode {
    Naive,
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
