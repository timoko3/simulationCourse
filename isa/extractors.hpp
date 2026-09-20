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
    Word extractLiRt(Word insn);

    Word extractImm(Word insn);
    Word extractSignedImm(Word insn, int len = 16);

    Word extractXorRd(Word insn);
    Word extractXorRt(Word insn);
    Word extractXorRs(Word insn);

    Word extractJIndex(Word insn);

    Word extractClsRd(Word insn);
    Word extractClsRs(Word insn);

    Word extractBextRd(Word insn);
    Word extractBextRs1(Word insn);
    Word extractBextRs2(Word insn);

    Word extractUsatRd(Word insn);
    Word extractUsatRs(Word insn);
    Word extractUsatImm(Word insn);

    Word extractLdRegBase(Word insn);
    Word extractLdRegRt(Word insn);
    Word extractLdRegRm(Word insn);

    Word extractSyscallCode(Word insn);
};

}
