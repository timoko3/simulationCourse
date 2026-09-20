#include <gtest/gtest.h>

#include "isa/extractors.hpp"

TEST(ExtractorTest, ExtractsBitRange)
{
    simulator::Extractor extractor;

    EXPECT_EQ(extractor.extract(0b110110u, 3, 2), 0b101u);
}

TEST(ExtractorTest, ExtractsOpcode)
{
    simulator::Extractor extractor;

    const simulator::Word instruction =
        (0b101010u << 26) | 0x03FFFFFFu;

    EXPECT_EQ(extractor.extractOpcode(instruction), 0b101010u);
}

TEST(ExtractorTest, ExtractsImmediate)
{
    simulator::Extractor extractor;

    EXPECT_EQ(extractor.extractImm(0xABCD1234u), 0x1234u);
}