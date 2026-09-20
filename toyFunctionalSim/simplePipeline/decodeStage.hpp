#pragma once

#include "isa/isa.hpp"
#include "isa/extractors.hpp"

namespace simulator{

class DecodeStage{
    Extractor extractor_;

    inline bool checkField(const Word& rawInstr, int len, int off, Word refVal);
    void requireField(Word rawInstr, int len, int off, Word expected);

    inline void decodeSpecial(const Word& rawInstr, Instruction& instr);

    inline void decodeAdd(const Word& rawInstr, Instruction& instr);
    inline void decodeAddi(const Word& rawInstr, Instruction& instr);
    inline void decodeBeq(const Word& rawInstr, Instruction& instr);
    inline void decodeBext(const Word& rawInstr, Instruction& instr);
    inline void decodeXor(const Word& rawInstr, Instruction& instr);
    inline void decodeCls(const Word& rawInstr, Instruction& instr);
    inline void decodeUsat(const Word& rawInstr, Instruction& instr);
    inline void decodeLdReg(const Word& rawInstr, Instruction& instr);
    inline void decodeLd(const Word& rawInstr, Instruction& instr);
    inline void decodeSt(const Word& rawInstr, Instruction& instr);
    inline void decodeLdp(const Word& rawInstr, Instruction& instr);
    inline void decodeSbit(const Word& rawInstr, Instruction& instr);

    inline void decodeLi(const Word& rawInstr, Instruction& instr);

    inline void decodeJ(const Word& rawInstr, Instruction& instr);

    inline void decodeSyscall(const Word& rawInstr, Instruction& instr);

public:
    Instruction instrDecode(const Word& rawInstr);
};

}
