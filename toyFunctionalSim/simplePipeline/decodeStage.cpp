#include "decodeStage.hpp"

#include <iostream>

namespace simulator{

Instruction 
DecodeStage::instrDecode(const Word& rawInstr){
    Instruction instr{};

    Opcode opc = static_cast<Opcode>(extractor_.extractOpcode(rawInstr));

    switch (opc){
        case Opcode::K_SPECIAL:
            decodeSpecial(rawInstr, instr);
            break;
        default:
            break;
    }

    return instr;
}

inline void 
DecodeStage::decodeSpecial(const Word& rawInstr, Instruction& instr){
    Funct6 funct6 = static_cast<Funct6>(extractor_.extractFunct6(rawInstr));

    switch (funct6){
        case Funct6::K_ADD:
            if(checkField(rawInstr, 5, 6, 0)){
                decodeAdd(rawInstr, instr);
            }
            else{
                std::cerr << "incorrect encoding\n";
                // TODO: exception;
            }
            break;
        
        default:
            break;
    }
}

inline void
DecodeStage::decodeAdd(const Word& rawInstr, Instruction& instr){
    instr.instrKind = InstrKind::I_ADD;

    instr.src1 = extractor_.extractRs(rawInstr);
    instr.src2 = extractor_.extractRt(rawInstr);

    instr.dst  = extractor_.extractAddRd(rawInstr);
}

inline bool
DecodeStage::checkField(const Word& rawInstr, int len, int off, Word refVal){
    return extractor_.extract(rawInstr, len, off) == refVal;
}

}