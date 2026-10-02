require_relative 'cpuArchitecture'
require_relative 'interAsm'
require_relative 'dataAsm'

require_relative 'elf/elfBuilder'

class Generator
    include RegFile
    include Isa
    include InstructionEncoding

    include InterAsmConstructions
    include DataAsmConstructions

    def initialize
        @instructions = []
        @output = []
        @outputElf = []
    end

    def generate
        @output = @instructions.map do |instruction|
            InstructionEncoding::RULES.fetch(instruction.instrKind).call(*instruction.operands)
        end
    end

    def makeOutputElf(loadAddress:, dataLoadAddress:)
        @outputElf = Elf::ElfBuilder.new(
            loadAddress: loadAddress,
            dataLoadAddress: dataLoadAddress
        ).build(getOutput, data: getDataOutput)
    end

    def save(path)
        File.binwrite(path, @output.pack("V*"))
    end

    def saveElf(path)
        File.binwrite(path, @outputElf)
    end

    def getOutput
        @output.pack("V*")
    end

private
    def emit(instrKind, *operands)
        @instructions << Instruction.new(
        instrKind: instrKind,
        operands: operands
        )
    end
end
