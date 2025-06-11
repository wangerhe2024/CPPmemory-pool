#pragma once
#include "Common.h"

namespace ww_memoryPool {

//线程本地缓存
class ThreadCache {

public:
  static ThreadCache *getInstance() {
    static thread_local ThreadCache instance;
    return &instance;
  }

  void allocate(size_t size);
  void deallocate(void *ptr, size_t size);

private:
  ThreadCache() = default;
  //璁数据中心获取缓存
  void *fetchFromCentralCache(size_t index);
  //归还内存到中心缓存
  void returnToCentralCache(void *start, size_t size, size_t bytes);

  //每个线程的自由链表数组
  std::array<void *, FREE_LIST_SIZE> freelist_;
};

} // namespace ww_memoryPool
