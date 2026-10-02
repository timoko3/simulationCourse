require_relative 'elf/elfConstants'

module DataAsmConstructions
    def data
        @data ||= "".b
    end

    def DATA_WORD(value = 0)
        unless value.between?(0, 0xFFFF_FFFF)
            raise RangeError, "DATA_WORD must fit into 32 bits"
        end

        address = Elf::Constants::MemoryMap::DATA_BASE + data.bytesize
        data << [value].pack("V")
        address
    end

    def getDataOutput
        data
    end
end
