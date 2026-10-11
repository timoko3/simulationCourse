require_relative '../../rubyDSL/microAsm'

assembleElf(ARGV.fetch(0)) do
    outer_iterations = 20
    inner_iterations = 60000
    batches_per_iteration = 8

    LI x0, 0
    LI x1, outer_iterations
    LI x2, 0
    LI x3, 0
    LI x4, 0
    LI x5, 0
    LI x7, 7
    LI x8, 11

    LABEL :Outer
    LI x6, inner_iterations

    LABEL :Inner
    batches_per_iteration.times do
        ADD x2, x2, x7
        ADD x3, x3, x8
        ADD x4, x4, x7
        ADD x5, x5, x8
    end

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
