#pragma once

#include "isa/isa.hpp"
#include "isa/extractors.hpp"

namespace simulator{

class DecodeStage{
    Extractor extractor_;

    inline bool checkField(const Word& rawInstr, int len, int off, Word refVal);

    inline void decodeSpecial(const Word& rawInstr, Instruction& instr);

    inline void decodeAdd(const Word& rawInstr, Instruction& instr);
public:
    Instruction instrDecode(const Word& rawInstr);
};

}