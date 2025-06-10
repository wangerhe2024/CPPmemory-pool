#include "../include/MemoryPool.h"

namespace ww_memoryPool {

MemoryPool::MemoryPool(size_t BlockSize)
    : BlockSize_(BlockSize), SlotSize_(0), firstBlock_(nullptr),
      curSlot_(nullptr), freeList_(nullptr), lastSlot_(nullptr) {}

MemoryPool::~MemoryPool() {
  //把连续的block删除
  Slot *cur = firstBlock_;
  while (cur) {
    Slot *next = cur->next;
    // 等同于free(reinterpret_cast<void*>(firstBlock_));
    // 转化为void指针，因为void类型不需要调用析构函数，只释放空间
    operator delete(reinterpret_cast<void *>(cur));
    cur = next;
  }
}

////////优化部分//////////
//实现无锁入队操作
bool MemoryPool::pushFreeList(Slot *slot) {
  while (true) {
    //获取当前头节点
    Slot *oldHead = freeList_.load(std::memory_order_relaxed);
    //将新节点的next指针指向当前头节点
    slot->next.store(oldHead, std::memory_order_relaxed);
    //尝试将新节点设置为头节点
    if (freeList_.compare_exchange_weak(oldHead, slot,
                                        std::memory_order_release,
                                        std::memory_order_relaxed)) {
      return true;
    }
  }
}

//实现无锁出队列操作
Slot *MemoryPool::popFreeList() {
  while (true) {
    Slot *oldHead = freeList_.load(std::memory_order_relaxed);
    if (oldHead == nullptr) {
      return nullptr; // 表示队列为空
    }

    //获取下一个节点
    Slot *newHead = oldHead->next.load(std::memory_order_relaxed);
    if (freeList_.compare_exchange_weak(oldHead, newHead,
                                        std::memory_order_acquire,
                                        std::memory_order_relaxed)) {
      return oldHead;
    }
  }
}

////////优化部分//////////

void MemoryPool::init(size_t size) {
  assert(size > 0);
  SlotSize_ = size;
  firstBlock_ = nullptr;
  curSlot_ = nullptr;
  freeList_.store(nullptr, std::memory_order_relaxed);
  lastSlot_ = nullptr;
}

void *MemoryPool::allocate() {
  //优先使用空闲链表中的内存槽
  //   if (freeList_ != nullptr) {
  //     {
  //       std::lock_guard<std::mutex> lock(mutexForFreeList_);
  //       if (freeList_ != nullptr) {
  //         Slot *temp = freeList_;
  //         freeList_ = freeList_->next;
  //         return temp;
  //       }
  //     }
  //   }

  //   Slot *temp;
  //   {
  //     std::lock_guard<std::mutex> lock(mutexForBlock_);
  //     if (curSlot_ >= lastSlot_) {
  //       //当前内存块已无内存槽可用，需要开辟一块新的内存
  //       allocateNewBlock();
  //     }

  //     temp = curSlot_;
  //     //此处不可直接curSlot_ += SlotSize_
  //     //因为CurSlot_是Slot*类型，所以需要除以Slot Size_再加1；
  //     curSlot_ += SlotSize_ / sizeof(Slot);
  //   }
  //   return temp;

  //优先使用空闲链表中的内存槽///优化！
  Slot *slot = popFreeList();
  if (slot != nullptr) {
    return slot;
  }

  //如果空闲链表为空，则重新分配新的内存
  std::lock_guard<std::mutex> lock(mutexForBlock_);
  if (curSlot_ >= lastSlot_) {
    //当前内存块已无内存槽可用，需要开辟一块新的内存
    allocateNewBlock();
  }

  Slot *result = curSlot_;
  curSlot_ =
      reinterpret_cast<Slot *>(reinterpret_cast<char *>(curSlot_) + SlotSize_);
  return result;
}

void MemoryPool::deallocate(void *ptr) {
  //   if (ptr) {
  //     //回收内存，将内存通过头插法插入到空闲链表中
  //     std::lock_guard<std::mutex> lock(mutexForFreeList_);
  //     reinterpret_cast<Slot *>(ptr)->next = freeList_;
  //     freeList_ = reinterpret_cast<Slot *>(ptr);
  //   }

  //优化
  if (!ptr)
    return;

  Slot *slot = static_cast<Slot *>(ptr);
  pushFreeList(slot);
}

void MemoryPool::allocateNewBlock() {
  // std::cout << "申请一块内存，SlotSize:" << SlotSize_ << std::endl;
  //头插法插入新的内存块
  void *newBlock = operator new(BlockSize_);
  reinterpret_cast<Slot *>(newBlock)->next = firstBlock_;
  firstBlock_ = reinterpret_cast<Slot *>(newBlock);

  char *body = reinterpret_cast<char *>(newBlock) + sizeof(Slot *);
  size_t paddingSize = padPointer(body, SlotSize_); //计算对齐所需填充的内存大小
  curSlot_ = reinterpret_cast<Slot *>(body + paddingSize);

  //超过该标记位置，则说明内存块已经没有内存槽可以使用，需要向系统申请新的内存块。
  lastSlot_ = reinterpret_cast<Slot *>(reinterpret_cast<size_t>(newBlock) +
                                       BlockSize_ - SlotSize_ + 1);

  freeList_ = nullptr;
}

//让指针对齐到槽大小的倍数的位置
size_t MemoryPool::padPointer(char *p, size_t align) {
  // aling是槽大小
  return (align - reinterpret_cast<size_t>(p)) %
         align; //存疑！！！！！！！！！！！！！！！！！！！
}

void HashBucket::initMemoryPool() {
  for (int i = 0; i < MEMORY_POOL_NUM; i++) {
    getMemoryPool(i).init((i + 1) * SLOT_BASE_SIZE);
  }
}

//单例模式
MemoryPool &HashBucket::getMemoryPool(int index) {
  static MemoryPool memoryPool[MEMORY_POOL_NUM];
  return memoryPool[index];
}
} // namespace ww_memoryPool