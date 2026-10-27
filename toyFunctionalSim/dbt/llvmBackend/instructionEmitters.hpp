#pragma once

#include <stdexcept>

#include <llvm/IR/Intrinsics.h>

#include "emitterTemplates.hpp"

namespace simulator::llvmEmitters {

inline void requireImmediateBitIndex(llvm::Value* value);

struct AddOperation {
    llvm::Value* operator()(llvm::IRBuilder<>& builder,
                            llvm::Value* lhs, llvm::Value* rhs) const {
        return builder.CreateAdd(lhs, rhs);
    }
};

struct XorOperation {
    llvm::Value* operator()(llvm::IRBuilder<>& builder,
                            llvm::Value* lhs, llvm::Value* rhs) const {
        return builder.CreateXor(lhs, rhs);
    }
};

struct CopyOperation {
    llvm::Value* operator()(llvm::IRBuilder<>&, llvm::Value* value) const {
        return value;
    }
};

inline void emitBeq(Context&, const Instruction&) {
    throw std::runtime_error("LLVM emitter BEQ is not implemented yet");
}

inline void emitLd(Context&, const Instruction&) {
    throw std::runtime_error("LLVM emitter LD is not implemented yet");
}

struct BitExtractOperation {
    llvm::Value* operator()(llvm::IRBuilder<>& builder,
                            llvm::Value* value, llvm::Value* mask) const {
        llvm::Value* result = builder.getInt32(0);
        llvm::Value* outputBit = builder.getInt32(1);

        for (unsigned bit = 0; bit < 32; ++bit) {
            auto* inputBit = builder.getInt32(Word{1} << bit);
            auto* selected = builder.CreateICmpNE(
                builder.CreateAnd(mask, inputBit), builder.getInt32(0));
            auto* set = builder.CreateICmpNE(
                builder.CreateAnd(value, inputBit), builder.getInt32(0));

            result = builder.CreateOr(result, builder.CreateSelect(
                builder.CreateAnd(selected, set), outputBit, builder.getInt32(0)));
            outputBit = builder.CreateSelect(selected,
                builder.CreateShl(outputBit, builder.getInt32(1)), outputBit);
        }

        return result;
    }
};

inline void emitJ(Context&, const Instruction&) {
    throw std::runtime_error("LLVM emitter J is not implemented yet");
}

inline void emitSt(Context&, const Instruction&) {
    throw std::runtime_error("LLVM emitter ST is not implemented yet");
}

inline void emitLdp(Context&, const Instruction&) {
    throw std::runtime_error("LLVM emitter LDP is not implemented yet");
}

struct CountLeadingOnesOperation {
    llvm::Value* operator()(llvm::IRBuilder<>& builder, llvm::Value* value) const {
        return builder.CreateIntrinsic(llvm::Intrinsic::ctlz, {value->getType()}, {builder.CreateNot(value), builder.getFalse()});
    }
};

struct SetBitOperation {
    llvm::Value* operator()(llvm::IRBuilder<>& builder, llvm::Value* bit) const {
        requireImmediateBitIndex(bit);

        return builder.CreateShl(builder.getInt32(1), bit);
    }
};

struct UnsignedSaturateOperation {
    llvm::Value* operator()(llvm::IRBuilder<>& builder,
                            llvm::Value* value, llvm::Value* width) const {
        requireImmediateBitIndex(width);

        auto* limit = builder.CreateSub(
            builder.CreateShl(builder.getInt32(1), width), builder.getInt32(1));

        return builder.CreateSelect(builder.CreateICmpULT(value, limit), value, limit);
    }
};

inline void emitLdReg(Context&, const Instruction&) {
    throw std::runtime_error("LLVM emitter LDREG is not implemented yet");
}

inline void emitSyscall(Context&, const Instruction&) {
    throw std::runtime_error("LLVM emitter SYSCALL is not implemented yet");
}

inline void requireImmediateBitIndex(llvm::Value* value) {
    const auto* index = llvm::dyn_cast<llvm::ConstantInt>(value);

    if (!index || index->getZExtValue() >= 32) {
        throw std::invalid_argument("Bit index or saturation width must be below 32");
    }
}

}
