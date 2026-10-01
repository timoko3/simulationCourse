require_relative "elfConstants"

module Elf
    class ElfBuilder
        def initialize(load_address:, endian: :little)
            @load_address = load_address
            configure_endian(endian)
        end

        def build(code)
            code = code.b
            code_offset = Constants::Header::Format64::SIZE +
                          Constants::ProgramHeader::Format64::SIZE
            file_size = code_offset + code.bytesize

            build_elf_header(code_offset) + build_program_header(file_size) + code
        end

    private
        def configure_endian(endian)
            case endian
                when :little
                    @word_format = "V"
                    @half_format = "v"
                    @xword_format = "Q<"
                    @data_encoding = Constants::Header::Data::LITTLE_ENDIAN
                when :big
                    @word_format = "N"
                    @half_format = "n"
                    @xword_format = "Q>"
                    @data_encoding = Constants::Header::Data::BIG_ENDIAN
                else
                    raise ArgumentError, "Unsupported endian: #{endian.inspect}"
            end
        end

        def build_elf_header(code_offset)
            ident = Constants::Header::MAGIC +
                    [
                        Constants::Header::Class::ELF64,
                        @data_encoding,
                        Constants::Header::Version::CURRENT,
                        Constants::Header::OsAbi::SYSTEM_V,
                        Constants::Header::OsAbiVersion::SYSTEM_V_VER
                    ].pack("C5") +
                    ("\0" * Constants::Header::IDENT_PADDING_SIZE)

            fields = [
                Constants::Header::Type::EXEC,
                Constants::Header::Machine::TIM_MACHINE,
                Constants::Header::Version::CURRENT,
                @load_address + code_offset,
                Constants::Header::Format64::SIZE,
                0,
                0,
                Constants::Header::Format64::SIZE,
                Constants::ProgramHeader::Format64::SIZE,
                Constants::ProgramHeader::COUNT,
                0,
                0,
                0
            ]

            format = format(
                Constants::Header::Format64::FIELDS_FORMAT,
                half: @half_format,
                word: @word_format,
                xword: @xword_format
            )

            ident + fields.pack(format)
        end

        def build_program_header(file_size)
            permissions = Constants::ProgramHeader::Permission::READ |
                          Constants::ProgramHeader::Permission::EXECUTE

            [
                Constants::ProgramHeader::Type::LOAD,
                permissions,
                0,
                @load_address,
                @load_address,
                file_size,
                file_size,
                Constants::ProgramHeader::DEFAULT_ALIGNMENT
            ].pack(
                format(
                    Constants::ProgramHeader::Format64::FIELDS_FORMAT,
                    word: @word_format,
                    xword: @xword_format
                )
            )
        end
    end
end
