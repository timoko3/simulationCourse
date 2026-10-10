#include <gtest/gtest.h>

#include "cpu.hpp"

TEST(CpuCacheTest, ReusesDecodedBlockOnHit)
{
    simulator::Memory memory;
    simulator::Cpu cpu;
    memory.write32(0, (0b000011u << 26) | 7u);

    const auto first = cpu.getBasicBlock(memory, 0);
    const auto cached = cpu.getBasicBlock(memory, 0);
    ASSERT_EQ(first.size(), 1u);
    ASSERT_EQ(cached.size(), 1u);
    EXPECT_EQ(cached[0].instrKind, first[0].instrKind);
    EXPECT_EQ(cached[0].src1, first[0].src1);
    EXPECT_EQ(cpu.getICache().getStats().amountMisses, 1u);
    EXPECT_EQ(cpu.getICache().getStats().amountHits, 1u);
}

TEST(CpuCacheTest, ReloadsBlockAfterEviction)
{
    simulator::Memory memory;
    simulator::Cpu cpu;
    for (simulator::Word index = 0; index < 2 * simulator::I_CACHE_SIZE; ++index) {
        const auto address = index * sizeof(simulator::Word);
        memory.write32(address, (0b000011u << 26) | index);
        const auto block = cpu.getBasicBlock(memory, address);
        ASSERT_EQ(block.size(), 1u);
        EXPECT_EQ(block[0].src1, index);
    }

    // A block from the HIR portion has been replaced by later cold accesses.
    const auto misses = cpu.getICache().getStats().amountMisses;
    const auto block = cpu.getBasicBlock(
        memory, simulator::I_CACHE_SIZE * sizeof(simulator::Word));
    ASSERT_EQ(block.size(), 1u);
    EXPECT_EQ(block[0].instrKind, simulator::InstrKind::I_J);
    EXPECT_EQ(block[0].src1, simulator::I_CACHE_SIZE);
    EXPECT_EQ(cpu.getICache().getStats().amountMisses, misses + 1);
}
