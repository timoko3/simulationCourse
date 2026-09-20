import struct
from pathlib import Path


def LI(rt, imm):
    return (0b011111 << 26) | (rt << 16) | (imm & 0xFFFF)


def ADD(rd, rs, rt):
    return (rs << 21) | (rt << 16) | (rd << 11) | 0b010101


def ADDI(rt, rs, imm):
    return (0b110000 << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF)


def BEQ(rs, rt, offset):
    return (0b111111 << 26) | (rs << 21) | (rt << 16) | (offset & 0xFFFF)


def J(index):
    return (0b000011 << 26) | (index & 0x3FFFFFF)


def LD(rt, base, imm=0):
    return (0b001010 << 26) | (base << 21) | (rt << 16) | (imm & 0x3FFF)


def ST(rt, base, imm=0):
    return (0b000110 << 26) | (base << 21) | (rt << 16) | (imm & 0x3FFF)


def SYSCALL(code=0):
    return ((code & 0xFFFFF) << 6) | 0b010100


if __name__ == "__main__":
    START = 10
    END = 18

    program = [
        LI(1, 5),
        LI(2, 0),
        LI(3, 0),
        LI(4, 0),
        BEQ(1, 4, END - 4),
        LI(3, 1),
        LI(4, 1),
        BEQ(1, 4, END - 7),
        LI(4, 1),
        LI(6, 1024),
        BEQ(4, 1, END - START),
        ADD(5, 2, 3),
        ST(3, 6),
        LD(2, 6),
        ST(5, 6),
        LD(3, 6),
        ADDI(4, 4, 1),
        J(START),
        LI(9, 60),
        SYSCALL(),
    ]

    data = struct.pack(f"<{len(program)}I", *program)
    output = Path(__file__).resolve().parent / "tests" / "fibonacci.bin"
    output.write_bytes(data)
    print(f"Создан {output}")
