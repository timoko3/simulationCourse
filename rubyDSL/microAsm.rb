require_relative 'generator'

def assembleElf(outputPath = "test.elf", &block)
    generator = Generator.new

    generator.instance_eval(&block)
    
    generator.replaceLabels
    
    generator.generate

    generator.makeOutputElf(load_address: 0x1000)

    generator.saveElf(outputPath)
end 
