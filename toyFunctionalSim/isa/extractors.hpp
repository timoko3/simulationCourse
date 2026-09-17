#pragma once 

#include "definitions.hpp"

namespace simulator{
    
class Extractor{

    inline Word extract(Word insn, int len, int off);
public:
    Word extractOpcode(Word insn);
};

}