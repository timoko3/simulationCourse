#pragma once 

#include "fetchStage.hpp"

namespace simulator{

class SimplePipeline{
    FetchStage fetch_;
public:
    void runSimplePipeline(Cpu& cpu);
};

}
