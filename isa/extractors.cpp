#include "extractors.hpp"

#include <cstddef>

namespace simulator{

constexpr size_t REGISTER_BITS_SIZE = 5;

Word
Extractor::extract(Word insn, int len, int off){
    return (insn >> off) & ( (1 << len) - 1 );
}

Word 
Extractor::extractOpcode(Word insn){
    return extract(insn, 6, 26);
}

Word 
Extractor::extractFunct6(Word insn){
    return extract(insn, 6, 0);
}

Word 
Extractor::extractFunct2(Word insn){
    return extract(insn, 2, 14);
}

Word 
Extractor::extractRs(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 21);
}

Word 
Extractor::extractRt(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 16);
}

Word 
Extractor::extractAddRd(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 11);
}

Word 
Extractor::extractXorRd(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 11);
}

Word 
Extractor::extractXorRt(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 16);
}

Word 
Extractor::extractXorRs(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 21);
}

Word 
Extractor::extractLiRt(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 16);
}

Word 
Extractor::extractImm(Word insn){
    return extract(insn, 16, 0);
}

Word
Extractor::extractSignedImm(Word insn, int len){
    const Word imm = extract(insn, len, 0);
    const Word signBit = Word{1} << (len - 1);
    return (imm & signBit) != 0 ? imm | (~Word{0} << len) : imm;
}

Word 
Extractor::extractSyscallCode(Word insn){
    return extract(insn, 20, 6);
}

Word
Extractor::extractJIndex(Word insn){
    return extract(insn, 26, 0);
}

Word
Extractor::extractClsRd(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 21);
}

Word
Extractor::extractClsRs(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 16);
}

Word
Extractor::extractBextRd(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 21);
}

Word
Extractor::extractBextRs1(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 16);
}

Word
Extractor::extractBextRs2(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 11);
}

Word
Extractor::extractUsatRd(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 21);
}

Word
Extractor::extractUsatRs(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 16);
}

Word
Extractor::extractUsatImm(Word insn){
    return extract(insn, 5, 11);
}

Word
Extractor::extractLdRegBase(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 21);
}

Word
Extractor::extractLdRegRt(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 16);
}

Word
Extractor::extractLdRegRm(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 0);
}

}
