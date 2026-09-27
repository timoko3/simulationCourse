#pragma once

#include "isa/isa.hpp"
#include "isa/extractors.hpp"

namespace simulator{

class DecodeStage{
    Extractor extractor_;

    inline bool checkField(Word rawInstr, int len, int off, Word refVal);
    void requireField(Word rawInstr, int len, int off, Word expected);

    inline void decodeSpecial(Word rawInstr, Instruction& instr);

    inline void decodeAdd(Word rawInstr, Instruction& instr);
    inline void decodeAddi(Word rawInstr, Instruction& instr);
    inline void decodeBeq(Word rawInstr, Instruction& instr);
    inline void decodeBext(Word rawInstr, Instruction& instr);
    inline void decodeXor(Word rawInstr, Instruction& instr);
    inline void decodeCls(Word rawInstr, Instruction& instr);
    inline void decodeUsat(Word rawInstr, Instruction& instr);
    inline void decodeLdReg(Word rawInstr, Instruction& instr);
    inline void decodeLd(Word rawInstr, Instruction& instr);
    inline void decodeSt(Word rawInstr, Instruction& instr);
    inline void decodeLdp(Word rawInstr, Instruction& instr);
    inline void decodeSbit(Word rawInstr, Instruction& instr);

    inline void decodeLi(Word rawInstr, Instruction& instr);

    inline void decodeJ(Word rawInstr, Instruction& instr);

    inline void decodeSyscall(Word rawInstr, Instruction& instr);

public:
    Instruction instrDecode(Word rawInstr);
};

}
