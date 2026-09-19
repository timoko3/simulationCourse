#pragma once 

#include "fetchStage.hpp"
#include "decodeStage.hpp"
#include "execStage.hpp"

namespace simulator{

struct SimplePipeline{
    FetchStage fetch_;
    DecodeStage decode_; 
    ExecStage exec_;    
};

}
