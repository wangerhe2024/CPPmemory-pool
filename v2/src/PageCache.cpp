#include "../include/PageCache.h"
#include <cstring>
#include <sys/mman.h>

namespace ww_memoryPool {

void *PageCache::allocateSpan(size_t numPages) {
  std::lock_guard<std::mutex> lock(mutex_);

  //查找合适的空闲span
  // lower_bound函数返回第一个大于等于numPages的元素迭代器
  auto it = freeSpans_.lower_bound(numPages);
  if (it != freeSpans_.end()) {
    Span *span = it->second;

    //将取出的span璁原有的空闲链表freeSpans_[it->first]中移除
    if (span->next) {
      freeSpans_[it->first] = span->next;
    } else {
      freeSpans_.erase(it);
    }

    //如果span大于需要的numPages则进行分割
    if (span->numPages > numPages) {
      Span *newSpan = new Span;
      newSpan->pageAddr =
          static_cast<char *>(span->pageAddr) + numPages * PAGE_SIZE;
      newSpan->numPages = span->numPages - numPages;
      newSpan->next = nullptr;

      //将超出部分放回空闲Span*列表头部
      auto &list = freeSpans_[newSpan->numPages];
      newSpan->next = list;
      list = newSpan;

      span->numPages = numPages;
    }

    //记录span信息用于回收
    spanMap_[span->pageAddr] = span;
    return span->pageAddr;
  }
}

} // namespace  ww_memoryPool