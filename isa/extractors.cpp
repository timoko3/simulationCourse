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

}