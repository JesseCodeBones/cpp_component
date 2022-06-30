#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>

#include "../logger/logger.hpp"

namespace MemoryManager {
static constexpr std::ptrdiff_t INVALID = ~static_cast<std::ptrdiff_t>(0);
static constexpr std::ptrdiff_t MIN_LEVEL = 5;
static constexpr std::ptrdiff_t MIN_SIZE = 1 << MIN_LEVEL;
static constexpr std::size_t FREE_LIST_SIZE = 7;
static constexpr std::ptrdiff_t MAX_LEVEL = MIN_LEVEL + FREE_LIST_SIZE - 1;
static constexpr std::ptrdiff_t MAX_SIZE = 1 << MAX_LEVEL;
static constexpr std::ptrdiff_t BUDDY_SIZE = 4096;

struct BuddyBlock {
  std::ptrdiff_t next_or_level;
};

static constexpr std::ptrdiff_t MAX_ALLOC_SIZE = MAX_SIZE - sizeof(BuddyBlock);

class BuddyAllocator {
public:
  BuddyAllocator() = delete;
  explicit BuddyAllocator(void *base) : base_(base) {
    static_cast<BuddyBlock *>(offset2ptr(0))->next_or_level = 1 << (FREE_LIST_SIZE - 1 + MIN_LEVEL);
    static_cast<BuddyBlock *>(offset2ptr(1 << (FREE_LIST_SIZE - 1 + MIN_LEVEL)))->next_or_level = INVALID;
  }

  static std::ptrdiff_t calc_level(std::ptrdiff_t size) { return std::__lg(std::max(size, MIN_SIZE) - 1) + 1; }
  static std::ptrdiff_t get_buddy(std::ptrdiff_t body, uint32_t level) { return body ^ (1 << level); }

  void *offset2ptr(std::ptrdiff_t offset) { return reinterpret_cast<void *>(static_cast<uint8_t *>(base_) + offset); }
  std::ptrdiff_t ptr2offset(void *ptr) { return static_cast<uint8_t *>(ptr) - static_cast<uint8_t *>(base_); }

  std::ptrdiff_t buddy_alloc(std::ptrdiff_t size) {
    const std::ptrdiff_t level = calc_level(size + sizeof(BuddyBlock));
    std::ptrdiff_t i = level - MIN_LEVEL;
    for (; i < FREE_LIST_SIZE; ++i) {
      if (freelists_[i] != INVALID) { break; }
    }
    if (i == FREE_LIST_SIZE) { return INVALID; }
    const auto block_offset = freelists_[i];
    const auto block_ptr = static_cast<BuddyBlock *>(offset2ptr(block_offset));
    // remove buddy into freelist
    freelists_[i] = block_ptr->next_or_level;
    block_ptr->next_or_level = level;
    while (i > level - MIN_LEVEL) {
      // add buddy into freelist
      i--;
      const auto buddy_block_offset = get_buddy(block_offset, i);
      const auto buddy_block_ptr = static_cast<BuddyBlock *>(offset2ptr(buddy_block_offset));
      buddy_block_ptr->next_or_level = freelists_[i];
      freelists_[i] = buddy_block_offset;
    }
    return block_offset + sizeof(BuddyBlock);
  }
  void buddy_free(std::ptrdiff_t offset) {
    std::ptrdiff_t block_offset = offset - sizeof(BuddyBlock);
    const auto block_ptr = static_cast<BuddyBlock *>(offset2ptr(block_offset));
    const auto level = block_ptr->next_or_level;
    auto i = level - MIN_LEVEL;
    for (; i < FREE_LIST_SIZE; i++) {
      const auto buddy_block_offset = get_buddy(block_offset, MIN_LEVEL + i);
      const auto buddy_block_ptr = static_cast<BuddyBlock *>(offset2ptr(buddy_block_offset));
      if (buddy_block_ptr->next_or_level <= MAX_LEVEL || i == FREE_LIST_SIZE - 1) {
        // buddy alloced
        block_ptr->next_or_level = freelists_[i];
        freelists_[i] = block_offset;
        break;
      } else {
        // buddy free, remove from freelist
        auto free = freelists_[i];
        auto update_ptr = &freelists_[i];
        while (free != INVALID) {
          auto free_ptr = static_cast<BuddyBlock *>(offset2ptr(free));
          if (free == buddy_block_offset) {
            *update_ptr = free_ptr->next_or_level;
            break;
          }
          free = free_ptr->next_or_level;
          update_ptr = &(free_ptr->next_or_level);
        }
      }
    }
  }

  void *getBase() const { return base_; }

private:
  // 32 64 128 256 512 1024 2048
  std::array<std::ptrdiff_t, FREE_LIST_SIZE> freelists_{INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, 0};
  void *base_;
};

class MemoryManager {
public:
  MemoryManager() : size_(0), base_(nullptr) {}
  explicit MemoryManager(std::ptrdiff_t page) : size_(page * 64 * 1024), base_(std::malloc(size_)) { _init(); }
  MemoryManager(MemoryManager const &mm) { _assign(mm); }
  MemoryManager &operator=(MemoryManager const &mm) {
    _clean();
    _assign(mm);
    return *this;
  }
  MemoryManager(MemoryManager &&mm) noexcept { _swap(std::move(mm)); }
  MemoryManager &operator=(MemoryManager &&mm) noexcept {
    _swap(std::move(mm));
    return *this;
  }
  ~MemoryManager() { _clean(); }

  std::ptrdiff_t mm_alloc(std::ptrdiff_t size) {
    if (size > MAX_ALLOC_SIZE) {
      // TODO
      assert(false);
    }
    for (std::size_t i = 0; i < buddies_.size(); i++) {
      auto offset = buddies_[i].buddy_alloc(size);
      if (offset != INVALID) { return i * BUDDY_SIZE + offset; }
    }
    extense();
    auto last = buddies_.rbegin();
    return (buddies_.size() - 1) * BUDDY_SIZE + last->buddy_alloc(size);
  }
  void mm_free(std::ptrdiff_t offset) { buddies_[offset / BUDDY_SIZE].buddy_free(offset % BUDDY_SIZE); }

  std::ptrdiff_t mm_realloc(std::ptrdiff_t offset, std::ptrdiff_t size) {
    // TODO
    return offset;
  }

private:
  void _clean() {
    if (base_ != nullptr) { std::free(base_); }
  }
  void _assign(MemoryManager const &mm) {
    if (&mm == this) { return; }
    base_ = std::malloc(mm.size_);
    size_ = mm.size_;
    std::memcpy(base_, mm.base_, size_);
  }
  void _swap(MemoryManager &&mm) noexcept {
    if (&mm == this) { return; }
    std::swap(base_, mm.base_);
    std::swap(size_, mm.size_);
  }

  void _init() { buddies_.emplace_back(base_); }

  void extense() { buddies_.emplace_back(static_cast<uint8_t *>(base_) + buddies_.size() * 4096); }

private:
  std::ptrdiff_t size_;
  void *base_;

  std::vector<BuddyAllocator> buddies_{};
};
}; // namespace MemoryManager