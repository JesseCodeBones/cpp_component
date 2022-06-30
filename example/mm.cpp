#include "../src/logger/logger.hpp"
#include "../src/memory_manager/memory_manager.hpp"
#include <cassert>
#include <cstddef>
#include <iostream>
#include <random>
#include <vector>

int main() {
  MemoryManager::MemoryManager mm{10000};
  std::random_device device{};
  std::default_random_engine e{device()};
  std::uniform_int_distribution<std::ptrdiff_t> distrib{0, 256};
  for (;;) {
    std::vector<std::ptrdiff_t> offsets{};
    for (auto i = 0; i < 100; i++) {
      auto size = distrib(e);
      auto offset = mm.mm_alloc(size);
      Logger::logInfo("alloc", size, "in", offset - 8);
      offsets.emplace_back(offset);
    }
    for (auto offset : offsets) {
      mm.mm_free(offset);
      Logger::logInfo("free", offset - 8);
    }
  }

  return 0;
}