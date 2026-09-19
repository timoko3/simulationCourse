#pragma once

#include "definitions.hpp"

namespace simulator{

enum class Opcode : std::uint8_t {
    K_UNKNOWN = 0b0, 
    K_BEQ     = 0b111111,
    K_ADDI    = 0b110000,
    K_LD      = 0b001010,
    K_LI      = 0b011111,
    K_J       = 0b000011,
    K_ST      = 0b000110,   
    K_LDP     = 0b111010,  
    k_SBIT    = 0b101110,
    K_USAT    = 0b010110,
    K_LD_REG  = 0b110101,
};

enum class Funct6 : std::uint8_t {
    K_ADD     = 0b010101,
    K_BEXT    = 0b111110,
    K_CLS     = 0b001011,
    k_XOR     = 0b011100,
    K_SYSCALL = 0b010100
};

struct Instruction {
    Opcode opc{};
    Word src1{}, src2{}, dst{};
};

}