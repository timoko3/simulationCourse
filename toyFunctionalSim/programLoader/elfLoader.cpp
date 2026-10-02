#include "elfLoader.hpp"

#include <elfio/elfio.hpp>

namespace simulator{

void
ElfLoader::loadProgram(const std::filesystem::path& filePath, Memory& memory){
    ELFIO::elfio elf;

    if (!elf.load(filePath.string())) {
            throw std::runtime_error("Cannot load ELF file: " + filePath.string());
    }

    std::cout << "Class: "
          << static_cast<unsigned>(elf.get_class())
          << '\n';

    std::cout << "Encoding: "
            << static_cast<unsigned>(elf.get_encoding())
            << '\n';

    std::cout << "Type: "
            << elf.get_type()
            << '\n';

    std::cout << "Machine: "
            << elf.get_machine()
            << '\n';

    std::cout << "Entry: 0x"
            << std::hex
            << elf.get_entry()
            << '\n';
}    

}