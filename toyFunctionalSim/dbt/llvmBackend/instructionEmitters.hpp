#pragma once

#include <llvm/IR/Intrinsics.h>

#include "emitterTemplates.hpp"

namespace simulator::llvmEmitters {

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

inline void emitBeq(Context& ctx, const Instruction& instruction) {
    auto* lhs = loadReg(ctx, instruction.src1);
    auto* rhs = loadReg(ctx, instruction.src2);
    auto* condition = ctx.builder.CreateICmpEQ(lhs, rhs);

    auto* offset = immediate(ctx, instruction.dst << 2);
    auto* target = ctx.builder.CreateAdd(loadPc(ctx), offset);

    ctx.nextPc = ctx.builder.CreateSelect(condition, target, ctx.nextPc);
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

inline void emitJ(Context& ctx, const Instruction& instruction) {
    auto* region = ctx.builder.CreateAnd(loadPc(ctx), ctx.builder.getInt32(0xF0000000));
    auto* target = immediate(ctx, instruction.src1 << 2);

    ctx.nextPc = ctx.builder.CreateOr(region, target);
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

inline void emitSyscall(Context& ctx, const Instruction& instruction) {
    emitGuestSyscall(ctx, immediate(ctx, instruction.src1));
}

}
