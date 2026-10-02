#pragma once 

#include <cstdint>

namespace simulator::elfConst{

constexpr std::uint16_t EM_TIM =
    29u * 36u * 36u +
    18u * 36u +
    22u;
static_assert(EM_TIM == 0x956Eu);
    
}