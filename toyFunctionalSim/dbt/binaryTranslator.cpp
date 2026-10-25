#include "binaryTranslator.hpp"

#include <stdexcept>

namespace simulator {

std::unique_ptr<HostBasicBlock> BinaryTranslator::translate(Addr pc, const BasicBlock& block)
{
    throw std::runtime_error("Binary translation is not implemented yet");
}

}
