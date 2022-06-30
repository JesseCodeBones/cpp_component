#include "../src/logger/logger.hpp"
#include "../src/memory_manager/memory_manager.hpp"
#include <cstddef>
#include <iostream>
#include <random>
#include <vector>

int main() {
  MemoryManager::MemoryManager mm{100000};
  std::random_device device{};
  std::default_random_engine e{device()};
  std::uniform_int_distribution<std::ptrdiff_t> distrib{0, 2000};
  for (;;) {
    std::vector<std::ptrdiff_t> offsets{};
    for (auto i = 0; i < 10; i++) {
      auto size = distrib(e);
      auto offset = mm.mm_alloc(size);
      Logger::logDebug("alloc", size, "in", offset);
      offsets.emplace_back(offset);
    }
    for (auto offset : offsets) {
      Logger::logDebug("free", offset);
      mm.mm_free(offset);
    }
  }

  return 0;
}