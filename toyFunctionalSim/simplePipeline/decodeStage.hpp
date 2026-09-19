#pragma once

#include "isa/isa.hpp"
#include "isa/extractors.hpp"

namespace simulator{

class DecodeStage{
    Extractor extractor_;

    inline void decodeUnknown(const Word& rawInstr, Instruction& instr);
public:
    Instruction instrDecode(const Word& rawInstr);
};

}