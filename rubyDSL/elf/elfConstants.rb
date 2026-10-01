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
            COUNT = 1
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
    end
end
