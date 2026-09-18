#pragma once

#include "isa/definitions.hpp"

#include "cpu.hpp"

namespace simulator{

class FetchStage{
public:
    Word instrFetch(Cpu& cpu);   
};

}