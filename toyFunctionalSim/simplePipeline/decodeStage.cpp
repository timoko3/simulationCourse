#include "decodeStage.hpp"


namespace simulator{

Instruction 
DecodeStage::instrDecode(const Word& rawInstr){
    Instruction instr{.opc = static_cast<Opcode>(extractor_.extractOpcode(rawInstr))};

    switch (instr.opc){
        case Opcode::K_UNKNOWN:
            decodeUnknown(rawInstr, instr);
            break;
        
        default:
            break;
    }
}

inline void 
DecodeStage::decodeUnknown(const Word& rawInstr, Instruction& instr){
    Funct6 funct6 = static_cast<Funct6>(extractor_.extractFunct6(rawInstr));

    switch (funct6){
        case Funct6::K_ADD:
            /* code */
            break;
        
        default:
            break;
    }
}


}