require_relative 'generator'
require_relative 'elf/elfLayout'

def assembleElf(outputPath = "test.elf", &block)
    layout = Elf::ElfLayout.new(
        loadAddress: Elf::Constants::MemoryMap::CODE_BASE,
        dataLoadAddress: Elf::Constants::MemoryMap::DATA_BASE
    )

    generator = Generator.new

    generator.instance_eval(&block)
    
    generator.replaceLabels(textAddress: layout.textAddress)
    
    generator.generate

    generator.makeOutputElf(layout: layout)

    generator.saveElf(outputPath)
end 
