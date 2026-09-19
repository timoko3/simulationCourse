#pragma once

#include "definitions.hpp"

namespace simulator{

enum class Opcode : std::uint8_t {
    K_UNKNOWN = 0, 
    K_ADD,
    K_BEQ,
    K_ADDI,
    K_LD,
    K_BEXT,
    K_LI,
    K_J,
    K_ST,
    K_LDP,
    K_SBIT,
    K_USAT, 
    K_SYSCALL
};

struct Instruction {
    Opcode opc{};
    Word src1{}, src2{}, dst{};
};

}