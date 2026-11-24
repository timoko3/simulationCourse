#pragma once

#include <array>
#include <type_traits>

namespace simulator {

struct CpuState;
struct Memory;
struct BlockDescriptor;

struct ExitSlot {
    BlockDescriptor* target = nullptr;
};

struct BlockDescriptor {
    using EntryPoint = ExitSlot* (*)(CpuState*, Memory*, BlockDescriptor*);

    EntryPoint entry = nullptr;
    std::array<ExitSlot, 2> exits{};
};

static_assert(std::is_standard_layout_v<ExitSlot>);
static_assert(std::is_standard_layout_v<BlockDescriptor>);

}
