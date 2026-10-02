require_relative "elfConstants"
require_relative "elfLayout"

module Elf
    class ElfBuilder
        def initialize(layout:, endian: :little)
            @layout = layout
            @loadAddress = layout.loadAddress
            @dataLoadAddress = layout.dataLoadAddress
            configureEndian(endian)
        end

        def build(code, data: "".b)
            code = code.b
            data = data.b
            codeOffset = @layout.codeOffset

            textSegmentFileSize = codeOffset + code.bytesize
            dataOffset = alignUp(
                textSegmentFileSize,
                Constants::ProgramHeader::DEFAULT_ALIGNMENT
            )

            if @loadAddress + textSegmentFileSize > @dataLoadAddress
                raise ArgumentError, "Text segment overlaps the data segment"
            end

            stringTable = Constants::SectionHeader::Name::TABLE
            stringTableOffset = dataOffset + data.bytesize
            dataPadding = "\0" * (dataOffset - textSegmentFileSize)

            buildElfHeader(
                codeOffset,
                sectionTableOffset: @layout.sectionTableOffset,
                sectionCount: Constants::SectionHeader::COUNT
            ) +
            buildProgramHeaders(
                textFileSize: textSegmentFileSize,
                dataOffset: dataOffset,
                dataSize: data.bytesize
            ) +
            buildSectionHeaders(
                codeOffset: codeOffset,
                codeSize: code.bytesize,
                dataOffset: dataOffset,
                dataSize: data.bytesize,
                stringTableOffset: stringTableOffset,
                stringTableSize: stringTable.bytesize
            ) +
            code +
            dataPadding +
            data +
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
                @layout.textAddress,
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

        def buildProgramHeaders(textFileSize:, dataOffset:, dataSize:)
            textPermissions = Constants::ProgramHeader::Permission::READ |
                              Constants::ProgramHeader::Permission::EXECUTE
            dataPermissions = Constants::ProgramHeader::Permission::READ |
                              Constants::ProgramHeader::Permission::WRITE

            textHeader = [
                Constants::ProgramHeader::Type::LOAD,
                textPermissions,
                0,
                @loadAddress,
                @loadAddress,
                textFileSize,
                textFileSize,
                Constants::ProgramHeader::DEFAULT_ALIGNMENT
            ].pack(
                format(
                    Constants::ProgramHeader::Format64::FIELDS_FORMAT,
                    word: @wordFormat,
                    xword: @xwordFormat
                )
            )

            dataHeader = [
                Constants::ProgramHeader::Type::LOAD,
                dataPermissions,
                dataOffset,
                @dataLoadAddress,
                @dataLoadAddress,
                dataSize,
                dataSize,
                Constants::ProgramHeader::DEFAULT_ALIGNMENT
            ].pack(
                format(
                    Constants::ProgramHeader::Format64::FIELDS_FORMAT,
                    word: @wordFormat,
                    xword: @xwordFormat
                )
            )

            textHeader + dataHeader
        end

        def buildSectionHeaders(
            codeOffset:,
            codeSize:,
            dataOffset:,
            dataSize:,
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
                address: @layout.textAddress,
                offset: codeOffset,
                size: codeSize,
                link: 0,
                info: 0,
                alignment: 4,
                entrySize: 0
            )

            dataHeader = buildSectionHeader(
                nameOffset: Constants::SectionHeader::Name::DATA_OFFSET,
                type: Constants::SectionHeader::Type::PROGBITS,
                flags: Constants::SectionHeader::Flag::ALLOC |
                       Constants::SectionHeader::Flag::WRITE,
                address: @dataLoadAddress,
                offset: dataOffset,
                size: dataSize,
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

            nullHeader + textHeader + dataHeader + stringTableHeader
        end

        def alignUp(value, alignment)
            (value + alignment - 1) / alignment * alignment
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
