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

    def XOR(rd, rs, rt)
        emit(:XOR, rd, rs, rt)
    end

    def BEXT(rd, rs1, rs2)
        emit(:BEXT, rd, rs1, rs2)
    end

    def CLS(rd, rs)
        emit(:CLS, rd, rs)
    end

    def USAT(rd, rs, imm)
        emit(:USAT, rd, rs, imm)
    end

    def SBIT(rd, rs, imm)
        emit(:SBIT, rd, rs, imm)
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

    def LDREG(rt, base, rm)
        emit(:LDREG, rt, base, rm)
    end

    def LDP(rt, rt2, base, imm=0)
        emit(:LDP, rt, rt2, base, imm)
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
        XOR: ->(rd, rs, rt) {
            (rs << 21) | (rt << 16) | (rd << 11) | 0b011100
        },
        BEXT: ->(rd, rs1, rs2) {
            (rd << 21) | (rs1 << 16) | (rs2 << 11) | 0b111110
        },
        CLS: ->(rd, rs) {
            (rd << 21) | (rs << 16) | 0b001011
        },
        USAT: ->(rd, rs, imm) {
            (0b010110 << 26) | (rd << 21) | (rs << 16) | ((imm & 0x1F) << 11)
        },
        SBIT: ->(rd, rs, imm) {
            (0b101110 << 26) | (rd << 21) | (rs << 16) | ((imm & 0x1F) << 11)
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
        LDREG: ->(rt, base, rm) {
            (0b110101 << 26) | (base << 21) | (rt << 16) | (0b11 << 14) | rm
        },
        LDP: ->(rt, rt2, base, imm=0) {
            (0b111010 << 26) | (base << 21) | (rt << 16) | (rt2 << 11) | (imm & 0x7FF)
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
