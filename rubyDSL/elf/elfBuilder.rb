require_relative "elfConstants"

module Elf
    class ElfBuilder
        def initialize(loadAddress:, endian: :little)
            @loadAddress = loadAddress
            configureEndian(endian)
        end

        def build(code)
            headersEnd = Constants::Header::Format64::SIZE +
                         Constants::ProgramHeader::Format64::SIZE
            
            sectionTableOffset = headersEnd
            sectionTableSize = Constants::SectionHeader::COUNT *
                               Constants::SectionHeader::Format64::SIZE
            
            code = code.b
            codeOffset = headersEnd + sectionTableSize

            stringTable = Constants::SectionHeader::Name::TABLE
            stringTableOffset = codeOffset + code.bytesize
            
            segmentFileSize = stringTableOffset

            buildElfHeader(
                codeOffset,
                sectionTableOffset: sectionTableOffset,
                sectionCount: Constants::SectionHeader::COUNT
            ) +
            buildProgramHeader(segmentFileSize) +
            buildSectionHeaders(
                codeOffset: codeOffset,
                codeSize: code.bytesize,
                stringTableOffset: stringTableOffset,
                stringTableSize: stringTable.bytesize
            ) +
            code +
            stringTable
        end

    private
        def configureEndian(endian)
            case endian
                when :little
                    @wordFormat = "V"
                    @halfFormat = "v"
                    @xwordFormat = "Q<"
                    @dataEncoding = Constants::Header::Data::LITTLE_ENDIAN
                when :big
                    @wordFormat = "N"
                    @halfFormat = "n"
                    @xwordFormat = "Q>"
                    @dataEncoding = Constants::Header::Data::BIG_ENDIAN
                else
                    raise ArgumentError, "Unsupported endian: #{endian.inspect}"
            end
        end

        def buildElfHeader(codeOffset, sectionTableOffset:, sectionCount:)
            ident = Constants::Header::MAGIC +
                    [
                        Constants::Header::Class::ELF64,
                        @dataEncoding,
                        Constants::Header::Version::CURRENT,
                        Constants::Header::OsAbi::SYSTEM_V,
                        Constants::Header::OsAbiVersion::SYSTEM_V_VER
                    ].pack("C5") +
                    ("\0" * Constants::Header::IDENT_PADDING_SIZE)

            fields = [
                Constants::Header::Type::EXEC,
                Constants::Header::Machine::TIM_MACHINE,
                Constants::Header::Version::CURRENT,
                @loadAddress + codeOffset,
                Constants::Header::Format64::SIZE,
                sectionTableOffset,
                0,
                Constants::Header::Format64::SIZE,
                Constants::ProgramHeader::Format64::SIZE,
                Constants::ProgramHeader::COUNT,
                Constants::SectionHeader::Format64::SIZE,
                sectionCount,
                Constants::SectionHeader::STRING_TABLE_INDEX
            ]

            packFormat = format(
                Constants::Header::Format64::FIELDS_FORMAT,
                half: @halfFormat,
                word: @wordFormat,
                xword: @xwordFormat
            )

            ident + fields.pack(packFormat)
        end

        def buildProgramHeader(fileSize)
            permissions = Constants::ProgramHeader::Permission::READ |
                          Constants::ProgramHeader::Permission::EXECUTE

            [
                Constants::ProgramHeader::Type::LOAD,
                permissions,
                0,
                @loadAddress,
                @loadAddress,
                fileSize,
                fileSize,
                Constants::ProgramHeader::DEFAULT_ALIGNMENT
            ].pack(
                format(
                    Constants::ProgramHeader::Format64::FIELDS_FORMAT,
                    word: @wordFormat,
                    xword: @xwordFormat
                )
            )
        end

        def buildSectionHeaders(
            codeOffset:,
            codeSize:,
            stringTableOffset:,
            stringTableSize:
        )
            nullHeader = buildSectionHeader(
                nameOffset: 0,
                type: Constants::SectionHeader::Type::NULL,
                flags: 0,
                address: 0,
                offset: 0,
                size: 0,
                link: 0,
                info: 0,
                alignment: 0,
                entrySize: 0
            )

            textHeader = buildSectionHeader(
                nameOffset: Constants::SectionHeader::Name::TEXT_OFFSET,
                type: Constants::SectionHeader::Type::PROGBITS,
                flags: Constants::SectionHeader::Flag::ALLOC |
                       Constants::SectionHeader::Flag::EXECUTE,
                address: @loadAddress + codeOffset,
                offset: codeOffset,
                size: codeSize,
                link: 0,
                info: 0,
                alignment: 4,
                entrySize: 0
            )

            stringTableHeader = buildSectionHeader(
                nameOffset: Constants::SectionHeader::Name::SHSTRTAB_OFFSET,
                type: Constants::SectionHeader::Type::STRTAB,
                flags: 0,
                address: 0,
                offset: stringTableOffset,
                size: stringTableSize,
                link: 0,
                info: 0,
                alignment: 1,
                entrySize: 0
            )

            nullHeader + textHeader + stringTableHeader
        end

        def buildSectionHeader(
            nameOffset:,
            type:,
            flags:,
            address:,
            offset:,
            size:,
            link:,
            info:,
            alignment:,
            entrySize:
        )
            fields = [
                nameOffset,
                type,
                flags,
                address,
                offset,
                size,
                link,
                info,
                alignment,
                entrySize
            ]

            packFormat = format(
                Constants::SectionHeader::Format64::FIELDS_FORMAT,
                word: @wordFormat,
                xword: @xwordFormat
            )

            fields.pack(packFormat)
        end
    end
end
