require_relative '../../rubyDSL/microAsm'

assembleElf(ARGV.fetch(0)) do
    outer_iterations = 5
    inner_iterations = 60000
    stages_per_iteration = 16

    LI x0, 0
    LI x1, outer_iterations
    LI x3, 1
    LI x9, 60

    LABEL :Outer
    LI x6, inner_iterations

    LABEL :Inner
    stages_per_iteration.times do |index|
        BEQ x0, x0, :"Taken#{index}"
        SYSCALL 1

        LABEL :"Taken#{index}"
        BEQ x0, x3, :Unexpected
        J :"Next#{index}"
        SYSCALL 1

        LABEL :"Next#{index}"
    end

    ADDI x6, x6, -1
    BEQ x6, x0, :Next
    J :Inner

    LABEL :Next
    ADDI x1, x1, -1
    BEQ x1, x0, :End
    J :Outer

    LABEL :End
    SYSCALL 0

    LABEL :Unexpected
    SYSCALL 1
end
