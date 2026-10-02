require_relative 'generator'

def assembleElf(outputPath = "test.elf", &block)
    generator = Generator.new

    generator.instance_eval(&block)
    
    generator.replaceLabels
    
    generator.generate

    generator.makeOutputElf(
        loadAddress: Elf::Constants::MemoryMap::CODE_BASE,
        dataLoadAddress: Elf::Constants::MemoryMap::DATA_BASE
    )

    generator.saveElf(outputPath)
end 
