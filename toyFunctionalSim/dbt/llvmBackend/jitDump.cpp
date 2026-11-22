#include "jitDump.hpp"

#include <stdexcept>
#include <system_error>
#include <utility>

#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Object/SymbolSize.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Format.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>

namespace simulator {

namespace {

llvm::Error writeFile(const std::filesystem::path& path, llvm::StringRef data);
void checkError(llvm::Error error);
void printGuestInstruction(llvm::raw_ostream& output, const Instruction& instruction, Addr pc);
const char* lookupTarget(void* data, std::uint64_t value, std::uint64_t* referenceType,
                         std::uint64_t referencePc, const char** referenceName);

}

LlvmJitDump::LlvmJitDump(const std::filesystem::path& directory, const llvm::Triple& triple)
    : triple_(triple.str())
{
    if (llvm::InitializeNativeTargetDisassembler()) {
        throw std::runtime_error("Failed to initialize the native LLVM disassembler");
    }

    disassembler_.reset(LLVMCreateDisasm(triple_.c_str(), &branchTarget_, 0, nullptr, &lookupTarget));
    if (!disassembler_) {
        throw std::runtime_error("Failed to create the LLVM disassembler for " + triple_);
    }
    if (triple.isX86()) {
        LLVMSetDisasmOptions(disassembler_.get(), LLVMDisassembler_Option_AsmPrinterVariant);
    }
    LLVMSetDisasmOptions(disassembler_.get(), LLVMDisassembler_Option_PrintImmHex);

    directory_ = std::filesystem::absolute(directory);
    std::filesystem::create_directories(directory_);
    llvm::SmallString<256> session;
    if (auto error = llvm::sys::fs::createUniqueDirectory((directory_ / "jit").string(), session)) {
        throw std::system_error(error, "Cannot create JIT dump directory");
    }
    directory_ = session.c_str();
    llvm::errs() << "JIT dump: " << directory_.string() << '\n';
}

void LlvmJitDump::begin(Addr pc, llvm::StringRef symbolName)
{
    guestPc_ = pc;
    symbolName_ = symbolName.str();
    codeSize_ = 0;
    blockDirectory_ = directory_ / ("block_" + std::to_string(pc) + "_" + std::to_string(translation_++));
    std::filesystem::create_directory(blockDirectory_);
}

void LlvmJitDump::saveGuest(const BasicBlock& block) const
{
    std::string guest;
    llvm::raw_string_ostream output(guest);
    output << "symbol: " << symbolName_ << "\nguest_pc: " << llvm::format_hex(guestPc_, 10)
           << "\ninstruction_count: " << block.size() << "\n\n";

    Addr pc = guestPc_;
    for (const auto& instruction : block) {
        output << llvm::format_hex(pc, 10) << "  ";
        printGuestInstruction(output, instruction, pc);
        output << '\n';
        pc += sizeof(Word);
    }

    checkError(writeFile(blockDirectory_ / "guest.txt", guest));
}

void LlvmJitDump::saveIr(const llvm::Module& module, llvm::StringRef stage) const
{
    std::string ir;
    llvm::raw_string_ostream output(ir);
    module.print(output, nullptr);
    checkError(writeFile(blockDirectory_ / (stage.str() + ".ll"), ir));
}

llvm::Expected<std::unique_ptr<llvm::MemoryBuffer>>
LlvmJitDump::saveObject(std::unique_ptr<llvm::MemoryBuffer> object)
{
    auto parsed = llvm::object::ObjectFile::createObjectFile(object->getMemBufferRef());
    if (!parsed) {
        return parsed.takeError();
    }

    for (const auto& [symbol, size] : llvm::object::computeSymbolSizes(**parsed)) {
        auto name = symbol.getName();
        if (!name) {
            return name.takeError();
        }
        if (*name == symbolName_) {
            codeSize_ = size;
            break;
        }
    }
    if (codeSize_ == 0) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                      "Cannot find native code size for %s", symbolName_.c_str());
    }
    if (auto error = writeFile(blockDirectory_ / "code.o", object->getBuffer())) {
        return error;
    }

    return std::move(object);
}

void LlvmJitDump::saveNative(const std::uint8_t* code)
{
    if (codeSize_ == 0) {
        throw std::runtime_error("JIT object was not captured for " + symbolName_);
    }

    checkError(writeFile(blockDirectory_ / "code.bin",
                        llvm::StringRef(reinterpret_cast<const char*>(code), codeSize_)));

    std::string assembly;
    llvm::raw_string_ostream output(assembly);
    output << "symbol: " << symbolName_ << "\nguest_pc: " << llvm::format_hex(guestPc_, 10)
           << "\ntarget: " << triple_ << "\nentry: " << llvm::format_hex(reinterpret_cast<std::uintptr_t>(code), 18)
           << "\ncode_bytes: " << codeSize_ << "\n\n";

    for (std::uint64_t offset = 0; offset < codeSize_;) {
        char instruction[256]{};
        const auto address = reinterpret_cast<std::uintptr_t>(code + offset);
        auto size = LLVMDisasmInstruction(disassembler_.get(), const_cast<std::uint8_t*>(code + offset),
                                         codeSize_ - offset, address, instruction, sizeof(instruction));
        if (size == 0) {
            size = 1;
            output << llvm::format_hex(address, 18) << "  .byte " << llvm::format_hex(code[offset], 4) << '\n';
        }
        else {
            output << llvm::format_hex(address, 18) << "  ";
            for (std::size_t index = 0; index < size; ++index) {
                output << llvm::format_hex_no_prefix(code[offset + index], 2) << ' ';
            }
            output << '\t' << instruction << '\n';
        }
        offset += size;
    }

    checkError(writeFile(blockDirectory_ / "code.asm", assembly));
}

namespace {

llvm::Error writeFile(const std::filesystem::path& path, llvm::StringRef data)
{
    std::error_code error;
    llvm::raw_fd_ostream output(path.string(), error, llvm::sys::fs::OF_None);
    if (error) {
        return llvm::createStringError(error, "Cannot write JIT dump %s", path.string().c_str());
    }

    output.write(data.data(), data.size());
    output.flush();
    if (output.has_error()) {
        error = output.error();
        output.clear_error();
        return llvm::createStringError(error, "Cannot write JIT dump %s", path.string().c_str());
    }
    return llvm::Error::success();
}

void checkError(llvm::Error error)
{
    if (error) {
        throw std::runtime_error(llvm::toString(std::move(error)));
    }
}

void printGuestInstruction(llvm::raw_ostream& output, const Instruction& instruction, Addr pc)
{
    switch (instruction.instrKind) {
        case InstrKind::I_ADD:
            output << "ADD x" << instruction.dst << ", x" << instruction.src1 << ", x" << instruction.src2;
            break;
        case InstrKind::I_BEQ:
            output << "BEQ x" << instruction.src1 << ", x" << instruction.src2 << ", "
                   << static_cast<std::int32_t>(instruction.dst) << " (target "
                   << llvm::format_hex(static_cast<Addr>(pc + (instruction.dst << 2)), 10) << ')';
            break;
        case InstrKind::I_ADDI:
            output << "ADDI x" << instruction.dst << ", x" << instruction.src1 << ", "
                   << static_cast<std::int32_t>(instruction.src2);
            break;
        case InstrKind::I_LD:
            output << "LD x" << instruction.dst << ", x" << instruction.src1 << ", "
                   << static_cast<std::int32_t>(instruction.src2);
            break;
        case InstrKind::I_BEXT:
            output << "BEXT x" << instruction.dst << ", x" << instruction.src1 << ", x" << instruction.src2;
            break;
        case InstrKind::I_LI:
            output << "LI x" << instruction.dst << ", " << instruction.src1;
            break;
        case InstrKind::I_J:
            output << "J " << llvm::format_hex((pc & 0xf0000000u) | (instruction.src1 << 2), 10);
            break;
        case InstrKind::I_ST:
            output << "ST x" << instruction.src2 << ", x" << instruction.src1 << ", "
                   << static_cast<std::int32_t>(instruction.dst);
            break;
        case InstrKind::I_LDP:
            output << "LDP x" << instruction.dst << ", x" << instruction.dst2 << ", x" << instruction.src1
                   << ", " << static_cast<std::int32_t>(instruction.src2);
            break;
        case InstrKind::I_CLS:
            output << "CLS x" << instruction.dst << ", x" << instruction.src1;
            break;
        case InstrKind::I_SBIT:
            output << "SBIT x" << instruction.dst << ", x" << instruction.src1 << ", " << instruction.src2;
            break;
        case InstrKind::I_USAT:
            output << "USAT x" << instruction.dst << ", x" << instruction.src1 << ", " << instruction.src2;
            break;
        case InstrKind::I_XOR:
            output << "XOR x" << instruction.dst << ", x" << instruction.src1 << ", x" << instruction.src2;
            break;
        case InstrKind::I_LDREG:
            output << "LDREG x" << instruction.dst << ", x" << instruction.src1 << ", x" << instruction.src2;
            break;
        case InstrKind::I_SYSCALL:
            output << "SYSCALL " << instruction.src1;
            break;
        default:
            output << "INVALID kind=" << static_cast<unsigned>(instruction.instrKind)
                   << " src1=" << instruction.src1 << " src2=" << instruction.src2
                   << " dst=" << instruction.dst << " dst2=" << instruction.dst2;
            break;
    }
}

const char* lookupTarget(void* data, std::uint64_t value, std::uint64_t* referenceType,
                         std::uint64_t, const char** referenceName)
{
    if (*referenceType != LLVMDisassembler_ReferenceType_In_Branch) {
        return nullptr;
    }

    auto& target = *static_cast<std::string*>(data);
    target = "0x" + llvm::utohexstr(value, true);
    *referenceType = LLVMDisassembler_ReferenceType_InOut_None;
    *referenceName = nullptr;
    return target.c_str();
}

}

}
