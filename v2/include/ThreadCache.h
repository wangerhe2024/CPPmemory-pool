#pragma once
#include "Common.h"

namespace ww_memoryPool {

// //线程本地缓存
// class ThreadCache {

// public:
//   static ThreadCache *getInstance() {
//     static thread_local ThreadCache instance;
//     return &instance;
//   }

//   void *allocate(size_t size);
//   void deallocate(void *ptr, size_t size);

// private:
//   ThreadCache() = default;
//   //   freeList_.fill(nullptr);
//   //   freeListSize_.fill(0);
//   //璁数据中心获取缓存
//   void *fetchFromCentralCache(size_t index);
//   //归还内存到中心缓存
//   void returnToCentralCache(void *start, size_t size, size_t bytes);

// private:
//   //每个线程的自由链表数组
//   std::array<void *, FREE_LIST_SIZE> freelist_;
//   //   std::array<size_t, FREE_LIST_SIZE> freeListSize_; // 自由链表大小统计
// };

// 线程本地缓存
class ThreadCache {
public:
  static ThreadCache *getInstance() {
    static thread_local ThreadCache instance;
    return &instance;
  }

  void *allocate(size_t size);
  void deallocate(void *ptr, size_t size);

private:
  ThreadCache() {
    // 初始化自由链表和大小统计
    freeList_.fill(nullptr);
    freeListSize_.fill(0);
  }

  // 从中心缓存获取内存
  void *fetchFromCentralCache(size_t index);
  // 归还内存到中心缓存
  void returnToCentralCache(void *start, size_t size);

  bool shouldReturnToCentralCache(size_t index);

private:
  // 每个线程的自由链表数组
  std::array<void *, FREE_LIST_SIZE> freeList_;
  std::array<size_t, FREE_LIST_SIZE> freeListSize_; // 自由链表大小统计
};

} // namespace ww_memoryPool
