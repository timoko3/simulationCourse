require_relative 'cpuArchitecture'

class Generator
    include RegFile
    include Isa
    include InstructionEncoding

    def initialize
        @instructions = []
        @output = []
    end

    def generate
        @output = @instructions.map do |instruction|
            InstructionEncoding::RULES.fetch(instruction.instrKind).call(*instruction.operands)
        end
    end

    def save(path)
        File.binwrite(path, @output.pack("V*"))
    end

private
    def emit(instrKind, *operands)
        @instructions << Instruction.new(
        instrKind: instrKind,
        operands: operands
        )
    end
end