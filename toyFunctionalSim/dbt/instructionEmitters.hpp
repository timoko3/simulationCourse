#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>

#include "isa/isa.hpp"

namespace simulator {

template<class Context>
class EmitterTable {
public:
    using Emitter = void (*)(Context&, const Instruction&);

private:
    std::array<Emitter, static_cast<std::size_t>(InstrKind::I_COUNT)> emitters_{};

public:
    void set(InstrKind kind, Emitter emitter) {
        emitters_.at(static_cast<std::size_t>(kind)) = emitter;
    }

    void emit(Context& context, const Instruction& instruction) const {
        const auto emitter = emitters_.at(static_cast<std::size_t>(instruction.instrKind));
        if (!emitter) {
            throw std::runtime_error("Instruction emitter is not implemented");
        }

        emitter(context, instruction);
    }
};

}
