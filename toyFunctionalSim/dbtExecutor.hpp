#pragma once

#include <cstddef>
#include <memory>
#include <unordered_map>

#include <cache/cacheLIRS.h>

#include "dbt/hostBasicBlock.hpp"
#include "dbt/binaryTranslator.hpp"

namespace simulator {

class Cpu;
struct Memory;

constexpr std::size_t T_CACHE_SIZE = 64;

class DbtExecutor {
    using TranslationCache =
        cache::CacheLIRS<std::shared_ptr<HostBasicBlock>, Addr>;

    TranslationCache translationCache_{T_CACHE_SIZE};
    std::unordered_map<Addr, std::size_t> executionCounts_;

    BinaryTranslator BinaryTranslator_;
public:
    explicit DbtExecutor(DbtLib backendType) : BinaryTranslator_(backendType) {}

    void run(Cpu& cpu, Memory& memory, std::size_t threshold);

};

}
