#include "../src/memory_manager//memory_manager.hpp"
#include <cstring>
#include <gtest/gtest.h>

using namespace MemoryManager;

TEST(test, buddy) {
  auto base = std::malloc(4096);
  BuddyAllocator allocator{base};

  EXPECT_EQ(BuddyAllocator::hight_bit(32), 5);
  EXPECT_EQ(BuddyAllocator::hight_bit(33), 6);
  EXPECT_EQ(BuddyAllocator::hight_bit(31), 5);

  EXPECT_EQ(BuddyAllocator::get_buddy(0, 10), 1024);

  std::free(base);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}