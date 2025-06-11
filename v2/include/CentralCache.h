#pragma once
#include "Common.h"
#include <mutex>

namespace ww_memoryPool {

class CenteralCache {

public:
  static CenteralCache &geiInstance() {
    static CenteralCache instance;
    return instance;
  }

  void *fetchRange(size_t index);
  void returnRange(void *start, size_t size, size_t bytes);

private:
  CenteralCache() {
    //初始化所有原子指针为nullptr
    for (auto &ptr : centralFreeList_) {
      ptr.store(nullptr, std::memory_order_relaxed);
    }
    //初始化所有锁
    for (auto &lock : locks_) {
      lock.clear();
    }
  }

  //璁页缓存获取与内存
  void *fetchFromPageCache(size_t size);

private:
  // 中心缓存的自由链表
  std::array<std::atomic<void *>, FREE_LIST_SIZE> centralFreeList_;

  // 用于同步的自旋锁
  std::array<std::atomic_flag, FREE_LIST_SIZE> locks_;
};

} // namespace ww_memoryPool