require_relative '../../rubyDSL/microAsm'

assembleElf(ARGV.fetch(0)) do
    passes = 64
    element_count = 256
    value_range = 128
    word_bytes = 4
    random = Random.new(0)
    values = Array.new(element_count) { random.rand(value_range) }
    golden_values = values.sort

    source = DATA_WORD values.first
    values.drop(1).each { |value| DATA_WORD value }

    golden = DATA_WORD golden_values.first
    golden_values.drop(1).each { |value| DATA_WORD value }

    buffer = DATA_WORD 0
    (element_count - 1).times { DATA_WORD 0 }

    LI x0, 0
    LI x1, passes
    LI x11, buffer
    LI x9, 60
    SBIT x8, x0, 31
    ADDI x10, x0, -1

    LABEL :Pass
    LI x2, source
    ADD x5, x11, x0
    LI x3, element_count

    LABEL :Copy
    LD x4, x2
    ST x4, x5
    ADDI x2, x2, word_bytes
    ADDI x5, x5, word_bytes
    ADDI x3, x3, -1
    BEQ x3, x0, :Sort
    J :Copy

    LABEL :Sort
    ADDI x2, x11, word_bytes
    LI x3, element_count - 1

    LABEL :Insert
    LD x4, x2
    ADD x5, x2, x0

    LABEL :Compare
    BEQ x5, x11, :Place
    LD x6, x5, -word_bytes
    # The sign of key - previous implements key < previous for these small values.
    XOR x7, x6, x10
    ADDI x7, x7, 1
    ADD x7, x4, x7
    BEXT x14, x7, x8
    BEQ x14, x0, :Place

    ST x6, x5
    ADDI x5, x5, -word_bytes
    J :Compare

    LABEL :Place
    ST x4, x5
    ADDI x2, x2, word_bytes
    ADDI x3, x3, -1
    BEQ x3, x0, :Check
    J :Insert

    LABEL :Check
    ADD x2, x11, x0
    LI x3, golden
    LI x5, element_count

    LABEL :CheckElement
    LD x4, x2
    LD x6, x3
    BEQ x4, x6, :Verified
    SYSCALL 1

    LABEL :Verified
    ADDI x2, x2, word_bytes
    ADDI x3, x3, word_bytes
    ADDI x5, x5, -1
    BEQ x5, x0, :NextPass
    J :CheckElement

    LABEL :NextPass
    ADDI x1, x1, -1
    BEQ x1, x0, :End
    J :Pass

    LABEL :End
    SYSCALL 0
end
