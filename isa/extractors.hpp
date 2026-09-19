#pragma once 

#include "definitions.hpp"

namespace simulator{
    
class Extractor{

    inline Word extract(Word insn, int len, int off);
public:
    Word extractOpcode(Word insn);
    Word extractFunct6(Word insn);
    Word extractFunct5(Word insn);
    Word extractFunct2(Word insn);
};

}