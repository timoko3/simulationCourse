module Elf
    module Constants
        module Header
            MAGIC = "\x7fELF".b.freeze
            IDENT_PADDING_SIZE = 7

            module Class
                ELF64 = 2
            end

            module Data
                LITTLE_ENDIAN = 1
                BIG_ENDIAN = 2
            end

            module Version
                CURRENT = 1
            end

            module OsAbi
                SYSTEM_V = 0
            end

            module OsAbiVersion
                SYSTEM_V_VER = 0
            end

            module Type
                EXEC = 2
            end

            module Machine
                # "TIM" encoded as three base-36 digits: T=29, I=18, M=22.
                TIM_MACHINE = 29 * (36**2) + 18 * 36 + 22
            end

            module Format64
                SIZE = 64
                FIELDS_FORMAT = "%<half>s2%<word>s%<xword>s3%<word>s%<half>s6".freeze
            end
        end

        module ProgramHeader
            COUNT = 2
            DEFAULT_ALIGNMENT = 0x1000

            module Format64
                SIZE = 56
                FIELDS_FORMAT = "%<word>s2%<xword>s6".freeze
            end

            module Type
                LOAD = 1
            end

            module Permission
                EXECUTE = 1
                WRITE = 2
                READ = 4
            end
        end

        module SectionHeader
            COUNT = 4
            STRING_TABLE_INDEX = 3

            module Format64
                SIZE = 64
                FIELDS_FORMAT = "%<word>s2%<xword>s4%<word>s2%<xword>s2".freeze
            end

            module Type
                NULL = 0
                PROGBITS = 1
                STRTAB = 3
                NOBITS = 8
            end

            module Flag
                WRITE = 0x1
                ALLOC = 0x2
                EXECUTE = 0x4
            end

            module Name
                TABLE = "\0.text\0.data\0.shstrtab\0".b.freeze
                TEXT_OFFSET = TABLE.index(".text")
                DATA_OFFSET = TABLE.index(".data")
                SHSTRTAB_OFFSET = TABLE.index(".shstrtab")
            end
        end

        module MemoryMap
            CODE_BASE = 0x1000
            DATA_BASE = 0x2000
        end
    end
end
