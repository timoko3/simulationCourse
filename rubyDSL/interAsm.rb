Label = Struct.new(:name, :value, keyword_init: true)

module InterAsmConstructions
    def labels
        @labels ||= {}
    end

    def LABEL(name)
        raise ArgumentError, "Duplicate label: #{name}" if labels.key?(name)

        labels[name] = @instructions.length
    end

    def replaceLabels(textAddress:)
        @instructions.each_with_index do |instruction, index|
        next unless [:J, :BEQ].include?(instruction.instrKind)

        instruction.operands.each_with_index do |operand, operand_index|
            next unless operand.is_a?(Symbol)

            target = getLabelPos(operand)

            instruction.operands[operand_index] =
            if instruction.instrKind == :BEQ
                target - index
            else
                (textAddress + target * 4) >> 2
            end
        end
        end
    end
        
private
    def getLabelPos(name)
        labels.fetch(name) do
            raise ArgumentError, "Unknown label: #{name}"
        end
    end

end 
