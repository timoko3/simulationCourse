#include "hostBasicBlock.hpp"
#include "blockLinker.hpp"

namespace simulator {

HostBasicBlock::~HostBasicBlock()
{
    if (linker_) {
        linker_->unlinkBlock(*this);
    }
}

}
