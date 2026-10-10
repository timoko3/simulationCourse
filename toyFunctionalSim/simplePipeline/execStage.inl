#pragma once

// Included by execStage.hpp after the ExecStage declaration.

#include <algorithm>
#include <stdexcept>

namespace simulator {

inline void
ExecStage::execAdd(CpuState& state, const Instruction& instr){
    Register rs = state.getReg(instr.src1);
    Register rt = state.getReg(instr.src2);

    state.setReg(instr.dst, rs + rt);

    state.pc += sizeof(Word);
}

inline void
ExecStage::execAddi(CpuState& state, const Instruction& instr){
    state.setReg(instr.dst, state.getReg(instr.src1) + instr.src2);
    state.pc += sizeof(Word);
}

inline void
ExecStage::execBeq(CpuState& state, const Instruction& instr){
    if (state.getReg(instr.src1) == state.getReg(instr.src2)) {
        state.pc += instr.dst << 2;
    } else {
        state.pc += sizeof(Word);
    }
}

inline void
ExecStage::execBext(CpuState& state, const Instruction& instr){
    const Word value = state.getReg(instr.src1);
    const Word mask = state.getReg(instr.src2);
    Word result = 0;
    Word outputBit = 1;

    for (Word inputBit = 1; inputBit != 0; inputBit <<= 1) {
        if ((mask & inputBit) != 0) {
            if ((value & inputBit) != 0) {
                result |= outputBit;
            }
            outputBit <<= 1;
        }
    }

    state.setReg(instr.dst, result);
    state.pc += sizeof(Word);
}

inline void
ExecStage::execXor(CpuState& state, const Instruction& instr){
    Register rs = state.getReg(instr.src1);
    Register rt = state.getReg(instr.src2);

    state.setReg(instr.dst, rs ^ rt);

    state.pc += sizeof(Word);
}

inline void
ExecStage::execCls(CpuState& state, const Instruction& instr){
    const Register value = state.getReg(instr.src1);
    Word count = 0;

    for (Word mask = Word{1} << 31; mask != 0 && (value & mask) != 0; mask >>= 1) {
        ++count;
    }

    state.setReg(instr.dst, count);
    state.pc += sizeof(Word);
}

inline void
ExecStage::execUsat(CpuState& state, const Instruction& instr){
    const Word limit = (Word{1} << instr.src2) - 1;
    state.setReg(instr.dst, std::min(state.getReg(instr.src1), limit));

    state.pc += sizeof(Word);
}

inline void
ExecStage::execLdReg(CpuState& state, Memory& memory, const Instruction& instr){
    const Addr addr = state.getReg(instr.src1) + state.getReg(instr.src2);
    requireAligned(addr);

    state.setReg(instr.dst, memory.read32(addr));

    state.pc += sizeof(Word);
}

inline void
ExecStage::execLd(CpuState& state, Memory& memory, const Instruction& instr){
    const Addr addr = state.getReg(instr.src1) + instr.src2;
    requireAligned(addr);
    state.setReg(instr.dst, memory.read32(addr));
    state.pc += sizeof(Word);
}

inline void
ExecStage::execSt(CpuState& state, Memory& memory, const Instruction& instr){
    const Addr addr = state.getReg(instr.src1) + instr.dst;
    requireAligned(addr);
    memory.write32(addr, state.getReg(instr.src2));
    state.pc += sizeof(Word);
}

inline void
ExecStage::execLdp(CpuState& state, Memory& memory, const Instruction& instr){
    const Addr addr = state.getReg(instr.src1) + instr.src2;
    requireAligned(addr);
    const Addr nextAddr = addr + Word{4};
    const Word first = memory.read32(addr);
    const Word second = memory.read32(nextAddr);
    state.setReg(instr.dst, first);
    state.setReg(instr.dst2, second);
    state.pc += sizeof(Word);
}

inline void
ExecStage::execSbit(CpuState& state, const Instruction& instr){
    state.setReg(instr.dst, Word{1} << instr.src2);
    state.pc += sizeof(Word);
}

inline void
ExecStage::execLi(CpuState& state, const Instruction& instr){
    state.setReg(instr.dst, instr.src1);

    state.pc += sizeof(Word);
}

inline void
ExecStage::execJ(CpuState& state, const Instruction& instr){

    state.pc = (state.pc & 0xF0000000) | (instr.src1 << 2);
}

inline void
ExecStage::execSyscall(CpuState& state, const Instruction& instr){
    syscallHandlers_.handle(state, instr.src1);
    state.pc += sizeof(Word);
}

inline void
ExecStage::requireAligned(Addr addr){
    if (addr % sizeof(Word) != 0) {
        throw std::runtime_error("MisalignedAccess");
    }
}

}
