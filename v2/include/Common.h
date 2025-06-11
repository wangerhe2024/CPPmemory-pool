#pragma once
#include <array>
#include <atomic>
#include <cstddef>

namespace ww_memeoryPool {
//对齐数字和大小定义
constexpr size_t ALIGNMENT = 8;
constexpr size_t MAX_BYTES = 256 * 1024; // 256KB
constexpr size_t FREE_LIST_SIZE = MAX_BYTES / ALIGNMENT;

//内存头部信息
struct BlockHeader {
  size_t size;       //内存块大小
  bool inUse;        // 使用标志
  BlockHeader *next; //指向下一个内存块
};

//大小类管理
class SizeClass {
public:
  static size_t roundUp(size_t bytes) {
    return (bytes + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
  }

  static size_t getIndex(size_t bytes) {
    //确保bytes至少为ALIGNMENT
    bytes = std::max(bytes, ALIGNMENT);
    return (bytes + ALIGNMENT - 1) / ALIGNMENT - 1;
    //因为空闲列表比编号璁0开始
  }
};

} // namespace ww_memeoryPool
