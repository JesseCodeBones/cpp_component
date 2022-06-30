#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>

#define private public
#include "../src/memory_manager//memory_manager.hpp"

using namespace MemoryManager;

TEST(buddy, hight_bit) {

  EXPECT_EQ(BuddyAllocator::hight_bit(32), 5);
  EXPECT_EQ(BuddyAllocator::hight_bit(33), 5);
  EXPECT_EQ(BuddyAllocator::hight_bit(31), 4);
}

TEST(buddy, get_buddy) {
  EXPECT_EQ(BuddyAllocator::get_buddy(0, 5), 32);
  EXPECT_EQ(BuddyAllocator::get_buddy(0, 10), 1024);
}
TEST(buddy, calc_level) {
  EXPECT_EQ(BuddyAllocator::calc_level(1), 5);
  EXPECT_EQ(BuddyAllocator::calc_level(32), 5);
  EXPECT_EQ(BuddyAllocator::calc_level(33), 6);
  EXPECT_EQ(BuddyAllocator::calc_level(64), 6);
}
TEST(buddy, alloc1) {
  auto base = std::malloc(4096);
  BuddyAllocator allocator{base};
  constexpr std::uintptr_t SIZE = 64 - 8;
  auto offset = allocator.buddy_alloc(SIZE) - 8;
  ASSERT_EQ(offset, 0);
  for (auto level = BuddyAllocator::calc_level(SIZE); level < MAX_LEVEL; level++) {
    auto buddy = BuddyAllocator::get_buddy(offset, level);
    ASSERT_EQ(allocator.is_alloc(buddy), false);
  }
  std::free(base);
}
TEST(buddy, alloc2) {
  auto base = std::malloc(4096);
  BuddyAllocator allocator{base};
  constexpr std::uintptr_t SIZE = 64 - 8;
  allocator.buddy_alloc(SIZE);
  auto offset = allocator.buddy_alloc(SIZE) - 8;
  ASSERT_EQ(offset, 64);
  auto buddy = BuddyAllocator::get_buddy(offset, BuddyAllocator::calc_level(SIZE));
  ASSERT_EQ(buddy, 0);
  ASSERT_EQ(allocator.is_alloc(buddy), true);
  std::free(base);
}

TEST(buddy, free1) {
  auto base = std::malloc(4096);
  BuddyAllocator allocator{base};
  auto offset = allocator.buddy_alloc(64 - 8);
  allocator.buddy_free(offset);
  ASSERT_EQ(allocator.freelists_[7], 0);
  std::free(base);
}

TEST(buddy, free2) {
  auto base = std::malloc(4096);
  BuddyAllocator allocator{base};
  auto offset1 = allocator.buddy_alloc(64 - 8);
  auto offset2 = allocator.buddy_alloc(64 - 8);
  allocator.buddy_free(offset1);
  allocator.buddy_free(offset2);
  ASSERT_EQ(allocator.freelists_[7], 0);
  std::free(base);
}

TEST(buddy, free3) {
  auto base = std::malloc(4096);
  BuddyAllocator allocator{base};
  auto offset1 = allocator.buddy_alloc(32 - 8);
  auto offset2 = allocator.buddy_alloc(32 - 8);
  auto offset3 = allocator.buddy_alloc(64 - 8);
  allocator.buddy_free(offset1);
  allocator.buddy_free(offset3);
  ASSERT_NE(allocator.freelists_[7], 0);
  std::free(base);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}