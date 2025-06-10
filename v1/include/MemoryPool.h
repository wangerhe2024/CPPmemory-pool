#pragma once //非标准但被绝大多数编译器支持的预处理指令，防止头文件被多重包含。

#include <atomic>  //提供原子操作（无锁线程安全操作）
#include <cassert> //提供 assert() 宏，用于程序运行时断言检查，通常用于开发/调试阶段检测不变量。
#include <cstdint> //提供固定宽度整数类型
#include <iostream> //标准输入输出流支持，用于调试或日志输出：
#include <memory>   //提供智能指针
#include <mutex>    //提供线程同步的互斥锁

namespace ww_memoryPool {
#define MEMORY_POOL_NUM 64
#define SLOT_BASE_SIZE 8
#define MAX_SLOT_SIZE 512

struct Slot {
  // Slot *next;
  std::atomic<Slot *> next; //优化
};

class MemoryPool {

public:
  MemoryPool(size_t BlockSize = 4096);
  ~MemoryPool();

  void init(size_t);
  void *allocate();
  void deallocate(void *);

private:
  void allocateNewBlock();
  size_t padPointer(char *p, size_t align);

  //使用CAS操作进行无锁入队和出队
  bool pushFreeList(Slot *slot); //优化
  Slot *popFreeList();           //优化

private:
  int BlockSize_;    //内存块大小
  int SlotSize_;     //槽大小
  Slot *firstBlock_; //指向内存池管理的首个实际内存块
  Slot *curSlot_;    //指向当前未被使用过的槽
  // Slot *freeList_;   //指向空闲的槽（被使用过后又释放掉的槽）
  std::atomic<Slot *> freeList_; //优化
  Slot *
      lastSlot_; // 作为当前内存块中最后能够存放元素的位置标识（超过该位置需要申请新的内存块）
  // std::mutex mutexForFreeList_; //保证FreeList_在多线程操作中的原子性；
  std::mutex
      mutexForBlock_; //保证多线程下避免不必要的重复开辟内存导致的浪费行为。
};

class HashBucket {

public:
  static void initMemoryPool();
  static MemoryPool &getMemoryPool(int index);
  static void *useMemory(size_t size) {
    if (size <= 0)
      return nullptr;
    if (size > MAX_SLOT_SIZE)
      return operator new(size);
    return getMemoryPool(((size + 7) / SLOT_BASE_SIZE) - 1).allocate();
  }

  static void freeMemory(void *ptr, size_t size) {

    if (!ptr)
      return;
    if (size > MAX_SLOT_SIZE) {
      operator delete(ptr);
      return;
    }

    getMemoryPool(((size + 7) / SLOT_BASE_SIZE) - 1).deallocate(ptr);
  }

  template <typename T, typename... Args> friend T *newElement(Args &&... args);

  template <typename T> friend void deleteElement(T *p);
};

template <typename T, typename... Args> T *newElement(Args &&... args) {
  T *p = nullptr;
  //根据元素大小选取合适的内存池分配内存
  if ((p = reinterpret_cast<T *>(HashBucket::useMemory(sizeof(T)))) != nullptr)
    new (p) T(std::forward<Args>(args)...);

  return p;
}

template <typename T> void deleteElement(T *p) {
  //析构对象
  if (p) {
    p->~T();
    //内存回收
    HashBucket::freeMemory(reinterpret_cast<void *>(p), sizeof(T));
  }
}

} // namespace ww_memoryPool
