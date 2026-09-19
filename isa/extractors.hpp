#pragma once 

#include "definitions.hpp"

namespace simulator{
    
class Extractor{
public:
    Word extract(Word insn, int len, int off);
    
    Word extractOpcode(Word insn);

    Word extractFunct6(Word insn);
    Word extractFunct2(Word insn);

    Word extractRs(Word insn);
    Word extractRt(Word insn);

    Word extractAddRd(Word insn);
};

}