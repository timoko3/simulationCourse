#pragma once

#include "isa/definitions.hpp"

#include "memory.hpp"

namespace simulator{

class FetchStage{
public:
    Word instrFetch(const Memory& memory, Register pc);   
};

}