require_relative 'generator'

def asm(outputPath = "test.bin", &block)
    generator = Generator.new

    generator.instance_eval(&block)

    generator.generate
    generator.save(outputPath)
end 
