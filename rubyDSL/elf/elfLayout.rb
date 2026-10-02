require_relative "elfConstants"

module Elf
    class ElfLayout
        attr_reader :loadAddress
        attr_reader :dataLoadAddress

        def initialize(loadAddress:, dataLoadAddress:)
            @loadAddress = loadAddress
            @dataLoadAddress = dataLoadAddress
        end

        def headersEnd
            Constants::Header::Format64::SIZE +
                Constants::ProgramHeader::COUNT *
                Constants::ProgramHeader::Format64::SIZE
        end

        def sectionTableOffset
            headersEnd
        end

        def sectionTableSize
            Constants::SectionHeader::COUNT *
                Constants::SectionHeader::Format64::SIZE
        end

        def codeOffset
            sectionTableOffset + sectionTableSize
        end

        def textAddress
            loadAddress + codeOffset
        end
    end
end
