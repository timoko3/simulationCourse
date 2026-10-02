#include "elfLoader.hpp"
#include "elfLoaderConst.hpp"

#include <elfio/elfio.hpp>

#include <iostream>
#include <stdexcept>

namespace simulator{

void
ElfLoader::loadProgram(const std::filesystem::path& filePath, Memory& memory, CpuState& state){
    ELFIO::elfio elf;

    if (!elf.load(filePath.string())){
        throw std::runtime_error("Cannot load ELF file: " + filePath.string());
    }

    validateElf(elf);

    state.pc = elf.get_entry();

    std::cout << elf.get_entry() << '\n';

    for (const auto& segment : elf.segments){
        std::cout
        << "index=" << segment->get_index()
        << ", type=" << segment->get_type()
        << ", offset=0x" << std::hex
        << segment->get_offset()
        << ", virtual address=0x"
        << segment->get_virtual_address()
        << ", physical address=0x"
        << segment->get_physical_address()
        << ", file size=0x"
        << segment->get_file_size()
        << ", memory size=0x"
        << segment->get_memory_size()
        << ", flags=0x"
        << segment->get_flags()
        << ", alignment=0x"
        << segment->get_align()
        << std::dec
        << '\n';

        if(segment->get_type() == ELFIO::PT_LOAD){
            loadSegment(*segment, memory);
        }
    }

}    

void
ElfLoader::validateElf(const ELFIO::elfio& elf){
    if (elf.get_class() != ELFIO::ELFCLASS64) {
        throw std::runtime_error(
            "Only 64-bit ELF files are supported");
    }

    if (elf.get_encoding() != ELFIO::ELFDATA2LSB) {
        throw std::runtime_error(
            "Only little-endian ELF files are supported");
    }

    if (elf.get_machine() != simulator::elfConst::EM_TIM) {
        throw std::runtime_error(
            "ELF file is not for TIM architecture");
    }
}

void
ElfLoader::loadSegment(const ELFIO::segment& segment, Memory& memory){
    const auto addr64 = segment.get_virtual_address();
    
    const auto fileSize64 = segment.get_file_size();
    const auto memorySize64 = segment.get_memory_size();

    if (memorySize64 < fileSize64) {
        throw std::runtime_error("ELF segment has invalid sizes");
    }

    if (addr64 > memory.data.size() ||
        memorySize64 > memory.data.size() - addr64) {
        throw std::runtime_error("ELF segment does not fit into memory");
    }

    const auto address    = static_cast<Addr>(addr64);
    const auto fileSize   = static_cast<std::size_t>(fileSize64);
    const auto memorySize = static_cast<std::size_t>(memorySize64);

    const char* data = segment.get_data();

    if (fileSize != 0 && data == nullptr) {
        throw std::runtime_error(
            "ELF segment data is missing");
    }

    if (fileSize != 0) {
        memory.loadData(
            address,
            reinterpret_cast<const Byte*>(data),
            fileSize);
    }

    if (memorySize > fileSize) {
        memory.fillData(
            static_cast<Addr>(address + fileSize),
            memorySize - fileSize);
    }
}

}
