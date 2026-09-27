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

    def BEQ(rs, rt, offset)
        emit(:BEQ, rs, rt, offset)
    end

    def ST(rt, base, imm=0)
        emit(:ST, rt, base, imm)
    end

    def LD(rt, base, imm=0)
        emit(:LD, rt, base, imm)
    end

    def ADDI(rt, rs, imm)
        emit(:ADDI, rt, rs, imm)
    end

    def J(index)
        emit(:J, index)
    end

    def SYSCALL(code=0)
        emit(:SYSCALL, code)
    end
end

module InstructionEncoding
    RULES = {
        LI: ->(rt, imm) {
            (0b011111 << 26) | (rt << 16) | (imm & 0xFFFF)
        },
        ADD: ->(rd, rs, rt) {
            (rs << 21) | (rt << 16) | (rd << 11) | 0b010101
        },
        BEQ: ->(rs, rt, offset) {
            (0b111111 << 26) | (rs << 21) | (rt << 16) | (offset & 0xFFFF)
        },
        ST: ->(rt, base, imm=0){
            (0b000110 << 26) | (base << 21) | (rt << 16) | (imm & 0x3FFF)
        },
        LD: ->(rt, base, imm=0){
            (0b001010 << 26) | (base << 21) | (rt << 16) | (imm & 0x3FFF)
        },
        ADDI: ->(rt, rs, imm){
            (0b110000 << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF)
        },
        J: ->(index){
            (0b000011 << 26) | (index & 0x3FFFFFF)
        },
        SYSCALL: ->(code){
            ((code & 0xFFFFF) << 6) | 0b010100
        }
    }.freeze
end