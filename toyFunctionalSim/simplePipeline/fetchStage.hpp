#pragma once

#include "isa/definitions.hpp"

namespace simulator{

class FetchStage{
public:
    Word instrFetch(const Memory& memory, Register pc);   
};

}