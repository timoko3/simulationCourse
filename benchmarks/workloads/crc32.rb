require 'zlib'
require_relative '../../rubyDSL/microAsm'

assembleElf(ARGV.fetch(0)) do
    passes = 64
    input = '123456789' * 128
    bits_per_byte = 8
    word_bytes = 4

    polynomial = DATA_WORD 0xEDB8_8320
    golden_crc = DATA_WORD Zlib.crc32(input)
    buffer = DATA_WORD input.getbyte(0)
    input.bytes.drop(1).each { |byte| DATA_WORD byte }

    LI x0, 0
    LI x1, passes
    LI x10, 1
    LI x9, 60

    ADDI x6, x0, -2
    ADDI x8, x0, -1
    LI x2, polynomial
    LD x7, x2
    LI x2, golden_crc
    LD x16, x2

    LABEL :Pass
    LI x11, buffer
    LI x12, input.bytesize
    ADD x15, x8, x0

    LABEL :Byte
    LD x13, x11
    XOR x15, x15, x13
    LI x5, bits_per_byte

    LABEL :Bit
    BEXT x14, x15, x10
    # Extracting bits 1..31 packs them into bits 0..30: logical right shift.
    BEXT x15, x15, x6
    BEQ x14, x0, :NextBit
    XOR x15, x15, x7

    LABEL :NextBit
    ADDI x5, x5, -1
    BEQ x5, x0, :NextByte
    J :Bit

    LABEL :NextByte
    ADDI x11, x11, word_bytes
    ADDI x12, x12, -1
    BEQ x12, x0, :Check
    J :Byte

    LABEL :Check
    XOR x15, x15, x8
    BEQ x15, x16, :NextPass
    SYSCALL 1

    LABEL :NextPass
    ADDI x1, x1, -1
    BEQ x1, x0, :End
    J :Pass

    LABEL :End
    SYSCALL 0
end
