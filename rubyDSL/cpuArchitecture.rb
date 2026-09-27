Instruction = Struct.new(:instrKind, :operands, keyword_init: true)

module RegFile
    32.times do |index|
        define_method("x#{index}") { index }
    end
end 

module Isa
    def LI(rt, imm)
        emit(:LI, rt, imm)
    end

    def ADD(rd, rs, rt)
        emit(:ADD, rd, rs, rt)
    end
end

module InstructionEncoding
    RULES = {
        LI: ->(rt, imm) {
            (0b011111 << 26) | (rt << 16) | (imm & 0xFFFF)
        }
        ADD: ->(rd, rs, rt) {
            (rs << 21) | (rt << 16) | (rd << 11) | 0b010101
        }
        
    }.freeze
end