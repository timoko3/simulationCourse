require_relative 'rubyDSL/microAsm'

assembleElf("tests/fibonacci.elf") do
    temporary = DATA_WORD 0

    LI x1, 5

    LI x2, 0
    LI x3, 0 
    LI x4, 0

    BEQ x1, x4, :End

    LI x3, 1
    LI x4, 1

    BEQ x1, x4, :End

    LI x4, 1
    LI x6, temporary

    LABEL :Start
    BEQ x4, x1, :End
        ADD x5, x2, x3
        ST x3, x6
        LD x2, x6
        ST x5, x6
        LD x3, x6
        ADDI x4, x4, 1
        J :Start
    LABEL :End
    LI x9, 60
    SYSCALL 0 
end 
