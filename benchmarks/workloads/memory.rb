require_relative '../../rubyDSL/microAsm'

assembleElf(ARGV.fetch(0)) do
    passes = 200
    buffer_words = 32768
    words_per_block = 16
    word_bytes = 4

    buffer = DATA_WORD 0
    (buffer_words - 1).times { |index| DATA_WORD(index + 1) }

    LI x0, 0
    LI x1, passes

    LABEL :Outer
    LI x2, buffer
    LI x6, buffer_words / words_per_block

    LABEL :Inner
    words_per_block.times do |index|
        offset = index * word_bytes
        LD x4, x2, offset
        ST x4, x2, offset
    end

    ADDI x2, x2, words_per_block * word_bytes
    ADDI x6, x6, -1
    BEQ x6, x0, :Next
    J :Inner

    LABEL :Next
    ADDI x1, x1, -1
    BEQ x1, x0, :End
    J :Outer

    LABEL :End
    LI x9, 60
    SYSCALL 0
end
