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
Extractor::extractLiRt(Word insn){
    return extract(insn, REGISTER_BITS_SIZE, 16);
}

Word 
Extractor::extractImm(Word insn){
    return extract(insn, 16, 0);
}

Word 
Extractor::extractSyscallCode(Word insn){
    return extract(insn, 20, 6);
}

}