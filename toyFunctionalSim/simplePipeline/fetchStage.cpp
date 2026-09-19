#include "fetchStage.hpp"

namespace simulator{

Word
FetchStage::instrFetch(const Memory& memory, Register pc){   
    return memory.data[pc];
}

}