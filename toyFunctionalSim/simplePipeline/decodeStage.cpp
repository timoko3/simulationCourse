#include "decodeStage.hpp"

#include <iostream>
#include <stdexcept>

namespace simulator{

Instruction 
DecodeStage::instrDecode(const Word& rawInstr){
    Instruction instr{};

    Opcode opc = static_cast<Opcode>(extractor_.extractOpcode(rawInstr));

    switch (opc){
        case Opcode::K_SPECIAL:
            decodeSpecial(rawInstr, instr);
            break;
        case Opcode::K_ADDI:
            decodeAddi(rawInstr, instr);
            break;
        case Opcode::K_BEQ:
            decodeBeq(rawInstr, instr);
            break;
        case Opcode::K_LI:
            requireField(rawInstr, 5, 21, 0);
            decodeLi(rawInstr, instr);
            break;
        case Opcode::K_J:
            decodeJ(rawInstr, instr);
            break;
        case Opcode::K_USAT:
            requireField(rawInstr, 11, 0, 0);
            decodeUsat(rawInstr, instr);
            break;
        case Opcode::K_LD_REG:
            requireField(rawInstr, 2, 14, 0b11);
            requireField(rawInstr, 9, 5, 0);
            decodeLdReg(rawInstr, instr);
            break;
        case Opcode::K_LD:
            requireField(rawInstr, 2, 14, 0);
            decodeLd(rawInstr, instr);
            break;
        case Opcode::K_ST:
            requireField(rawInstr, 2, 14, 0);
            decodeSt(rawInstr, instr);
            break;
        case Opcode::K_LDP:
            decodeLdp(rawInstr, instr);
            break;
        case Opcode::k_SBIT:
            requireField(rawInstr, 11, 0, 0);
            decodeSbit(rawInstr, instr);
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
            requireField(rawInstr, 5, 6, 0);
            decodeAdd(rawInstr, instr);
            break;
        case Funct6::K_XOR:
            requireField(rawInstr, 5, 6, 0);
            decodeXor(rawInstr, instr);
            break;
        case Funct6::K_BEXT:
            requireField(rawInstr, 5, 6, 0);
            decodeBext(rawInstr, instr);
            break;
        case Funct6::K_SYSCALL:
            decodeSyscall(rawInstr, instr);
            break;
        case Funct6::K_CLS:
            requireField(rawInstr, 10, 6, 0);
            decodeCls(rawInstr, instr);
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

inline void
DecodeStage::decodeAddi(const Word& rawInstr, Instruction& instr){
    instr.instrKind = InstrKind::I_ADDI;
    instr.src1 = extractor_.extractRs(rawInstr);
    instr.src2 = extractor_.extractSignedImm(rawInstr);
    instr.dst = extractor_.extractRt(rawInstr);
}

inline void
DecodeStage::decodeBeq(const Word& rawInstr, Instruction& instr){
    instr.instrKind = InstrKind::I_BEQ;
    instr.src1 = extractor_.extractRs(rawInstr);
    instr.src2 = extractor_.extractRt(rawInstr);
    instr.dst = extractor_.extractSignedImm(rawInstr);
}

inline void
DecodeStage::decodeBext(const Word& rawInstr, Instruction& instr){
    instr.instrKind = InstrKind::I_BEXT;
    instr.src1 = extractor_.extractBextRs1(rawInstr);
    instr.src2 = extractor_.extractBextRs2(rawInstr);
    instr.dst = extractor_.extractBextRd(rawInstr);
}

inline void
DecodeStage::decodeXor(const Word& rawInstr, Instruction& instr){
    instr.instrKind = InstrKind::I_XOR;

    instr.src1 = extractor_.extractXorRs(rawInstr);
    instr.src2 = extractor_.extractXorRt(rawInstr);

    instr.dst  = extractor_.extractXorRd(rawInstr);
}

inline void
DecodeStage::decodeCls(const Word& rawInstr, Instruction& instr){
    instr.instrKind = InstrKind::I_CLS;
    instr.src1 = extractor_.extractClsRs(rawInstr);
    instr.dst = extractor_.extractClsRd(rawInstr);
}

inline void
DecodeStage::decodeUsat(const Word& rawInstr, Instruction& instr){
    instr.instrKind = InstrKind::I_USAT;
    instr.src1 = extractor_.extractUsatRs(rawInstr);
    instr.src2 = extractor_.extractUsatImm(rawInstr);
    instr.dst = extractor_.extractUsatRd(rawInstr);
}

inline void
DecodeStage::decodeLdReg(const Word& rawInstr, Instruction& instr){
    instr.instrKind = InstrKind::I_LDREG;
    instr.src1 = extractor_.extractLdRegBase(rawInstr);
    instr.src2 = extractor_.extractLdRegRm(rawInstr);
    instr.dst = extractor_.extractLdRegRt(rawInstr);
}

inline void
DecodeStage::decodeLd(const Word& rawInstr, Instruction& instr){
    instr.instrKind = InstrKind::I_LD;
    instr.src1 = extractor_.extractRs(rawInstr);
    instr.src2 = extractor_.extractSignedImm(rawInstr, 14);
    instr.dst = extractor_.extractRt(rawInstr);
}

inline void
DecodeStage::decodeSt(const Word& rawInstr, Instruction& instr){
    instr.instrKind = InstrKind::I_ST;
    instr.src1 = extractor_.extractRs(rawInstr);
    instr.src2 = extractor_.extractRt(rawInstr);
    instr.dst = extractor_.extractSignedImm(rawInstr, 14);
}

inline void
DecodeStage::decodeLdp(const Word& rawInstr, Instruction& instr){
    instr.instrKind = InstrKind::I_LDP;
    instr.src1 = extractor_.extractRs(rawInstr);
    instr.src2 = extractor_.extractSignedImm(rawInstr, 11);
    instr.dst = extractor_.extractRt(rawInstr);
    instr.dst2 = extractor_.extract(rawInstr, 5, 11);
}

inline void
DecodeStage::decodeSbit(const Word& rawInstr, Instruction& instr){
    instr.instrKind = InstrKind::I_SBIT;
    instr.src1 = extractor_.extractRt(rawInstr);
    instr.src2 = extractor_.extract(rawInstr, 5, 11);
    instr.dst = extractor_.extractRs(rawInstr);
}

inline void
DecodeStage::decodeLi(const Word& rawInstr, Instruction& instr){
    instr.instrKind = InstrKind::I_LI;

    instr.src1 = extractor_.extractImm(rawInstr); 

    instr.dst  = extractor_.extractLiRt(rawInstr);
}

inline void
DecodeStage::decodeJ(const Word& rawInstr, Instruction& instr){
    instr.instrKind = InstrKind::I_J;

    instr.src1 = extractor_.extractJIndex(rawInstr); 
}

inline void
DecodeStage::decodeSyscall(const Word& rawInstr, Instruction& instr){
    instr.instrKind = InstrKind::I_SYSCALL;

    instr.src1 = extractor_.extractSyscallCode(rawInstr);
}

void DecodeStage::requireField(
    Word rawInstr, int len, int off, Word expected)
{
    if (!checkField(rawInstr, len, off, expected)) {
        throw std::runtime_error("Invalid instruction encoding");
    }
}

inline bool
DecodeStage::checkField(const Word& rawInstr, int len, int off, Word refVal){
    return extractor_.extract(rawInstr, len, off) == refVal;
}

}
