#include "../include/CentralCache.h"
#include "../include/PageCache.h"
#include <cassert>
#include <thread>

namespace ww_memoryPool {

static const size_t SPAN_PAGES = 8;

void *CentralCache::fetchRange(size_t index, size_t batchNum) {}

} // namespace ww_memoryPool
