require_relative 'generator'

def assembleElf(outputPath = "test.elf", &block)
    generator = Generator.new

    generator.instance_eval(&block)
    
    generator.replaceLabels
    
    generator.generate

    generator.makeOutputElf(loadAddress: 0x1000)

    generator.saveElf(outputPath)
end 
