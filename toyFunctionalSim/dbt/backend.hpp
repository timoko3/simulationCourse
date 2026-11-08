#pragma once 

#include <memory>

#include "hostBasicBlock.hpp"
#include "isa/isa.hpp"

namespace simulator {

class TranslationBackend{
public:
    virtual ~TranslationBackend() = default;

    virtual std::unique_ptr<HostBasicBlock> translate(Addr startPc, const BasicBlock& block) = 0;
};

}
