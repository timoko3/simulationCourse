#include "extractors.hpp"

namespace simulator{

inline Word
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
Extractor::extractFunct6(Word insn){
    return extract(insn, 5, 6);
}

Word 
Extractor::extractFunct2(Word insn){
    return extract(insn, 2, 14);
}



}