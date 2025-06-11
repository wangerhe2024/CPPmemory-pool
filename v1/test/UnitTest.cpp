// #include <iostream>
// #include <thread>
// #include <vector>

// #include "../include/MemoryPool.h"

// using namespace ww_memoryPool;

// // 测试用例
// class P1 {
//   int id_;
// };

// class P2 {
//   int id_[5];
// };

// class P3 {
//   int id_[10];
// };

// class P4 {
//   int id_[20];
// };

// // 单轮次申请释放次数 线程数 轮次
// void BenchmarkMemoryPool(size_t ntimes, size_t nworks, size_t rounds) {
//   std::vector<std::thread> vthread(nworks); // 线程池
//   size_t total_costtime = 0;
//   for (size_t k = 0; k < nworks; ++k) // 创建 nworks 个线程
//   {
//     vthread[k] = std::thread([&]() {
//       for (size_t j = 0; j < rounds; ++j) {
//         size_t begin1 = clock();
//         for (size_t i = 0; i < ntimes; i++) {
//           P1 *p1 = newElement<P1>(); // 内存池对外接口
//           deleteElement<P1>(p1);
//           P2 *p2 = newElement<P2>();
//           deleteElement<P2>(p2);
//           P3 *p3 = newElement<P3>();
//           deleteElement<P3>(p3);
//           P4 *p4 = newElement<P4>();
//           deleteElement<P4>(p4);
//         }
//         size_t end1 = clock();

//         total_costtime += end1 - begin1;
//       }
//     });
//   }
//   for (auto &t : vthread) {
//     t.join();
//   }
//   printf("%lu个线程并发执行%lu轮次，每轮次newElement&deleteElement "
//          "%lu次，总计花费：%lu ms\n",
//          nworks, rounds, ntimes, total_costtime);
// }

// void BenchmarkNew(size_t ntimes, size_t nworks, size_t rounds) {
//   std::vector<std::thread> vthread(nworks);
//   size_t total_costtime = 0;
//   for (size_t k = 0; k < nworks; ++k) {
//     vthread[k] = std::thread([&]() {
//       for (size_t j = 0; j < rounds; ++j) {
//         size_t begin1 = clock();
//         for (size_t i = 0; i < ntimes; i++) {
//           P1 *p1 = new P1;
//           delete p1;
//           P2 *p2 = new P2;
//           delete p2;
//           P3 *p3 = new P3;
//           delete p3;
//           P4 *p4 = new P4;
//           delete p4;
//         }
//         size_t end1 = clock();

//         total_costtime += end1 - begin1;
//       }
//     });
//   }
//   for (auto &t : vthread) {
//     t.join();
//   }
//   printf(
//       "%lu个线程并发执行%lu轮次，每轮次malloc&free %lu次，总计花费：%lu
//       ms\n", nworks, rounds, ntimes, total_costtime);
// }

// int main() {
//   HashBucket::initMemoryPool(); // 使用内存池接口前一定要先调用该函数
//   BenchmarkMemoryPool(100000, 1, 10); // 测试内存池
//   std::cout <<
//   "==============================================================="
//                "============"
//             << std::endl;
//   std::cout <<
//   "==============================================================="
//                "============"
//             << std::endl;
//   BenchmarkNew(100000, 1, 10); // 测试 new delete

//   return 0;
// }

#include <atomic>   // ✅ 新增
#include <chrono>   // ✅ 新增
#include <iostream> // ✅ 新增
#include <numeric>
#include <thread>
#include <vector>

#include "../include/MemoryPool.h"

using namespace ww_memoryPool;

// 测试用例
class P1 {
  int id_;
};

class P2 {
  int id_[5];
};

class P3 {
  int id_[10];
};

class P4 {
  int id_[20];
};

// 单轮次申请释放次数 线程数 轮次
// void BenchmarkMemoryPool(size_t ntimes, size_t nworks, size_t rounds) {
//   std::vector<std::thread> vthread(nworks); // 线程池
//   std::atomic<size_t> total_costtime(0);    // ✅ 改为线程安全累加
//   for (size_t k = 0; k < nworks; ++k) {
//     vthread[k] = std::thread([&]() {
//       for (size_t j = 0; j < rounds; ++j) {
//         auto begin1 = std::chrono::high_resolution_clock::now();
//         for (size_t i = 0; i < ntimes; i++) {
//           P1 *p1 = newElement<P1>();
//           deleteElement<P1>(p1);
//           P2 *p2 = newElement<P2>();
//           deleteElement<P2>(p2);
//           P3 *p3 = newElement<P3>();
//           deleteElement<P3>(p3);
//           P4 *p4 = newElement<P4>();
//           deleteElement<P4>(p4);
//         }
//         auto end1 = std::chrono::high_resolution_clock::now();
//         total_costtime +=
//             std::chrono::duration_cast<std::chrono::milliseconds>(end1 -
//             begin1)
//                 .count();
//       }
//     });
//   }
//   for (auto &t : vthread) {
//     t.join();
//   }
//   printf("%lu个线程并发执行%lu轮次，每轮次newElement&deleteElement "
//          "%lu次，总计花费：%lu ms\n",
//          nworks, rounds, ntimes, total_costtime.load());
// }

void BenchmarkMemoryPool(size_t ntimes, size_t nworks, size_t rounds) {
  std::vector<std::thread> vthread;
  std::vector<size_t> thread_times(nworks, 0);

  for (size_t k = 0; k < nworks; ++k) {
    vthread.emplace_back([&, k]() {
      for (size_t j = 0; j < rounds; ++j) {
        auto begin = std::chrono::steady_clock::now();
        for (size_t i = 0; i < ntimes; ++i) {
          auto *p1 = newElement<P1>();
          deleteElement<P1>(p1);
          auto *p2 = newElement<P2>();
          deleteElement<P2>(p2);
          auto *p3 = newElement<P3>();
          deleteElement<P3>(p3);
          auto *p4 = newElement<P4>();
          deleteElement<P4>(p4);
        }
        auto end = std::chrono::steady_clock::now();
        thread_times[k] +=
            std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
                .count();
      }
    });
  }

  for (auto &t : vthread)
    t.join();

  size_t total =
      std::accumulate(thread_times.begin(), thread_times.end(), size_t(0));
  printf("%lu个线程并发执行%lu轮次，每轮次newElement&deleteElement "
         "%lu次，总计花费：%lu ms\n",
         nworks, rounds, ntimes, total);
}

void BenchmarkNew(size_t ntimes, size_t nworks, size_t rounds) {
  std::vector<std::thread> vthread(nworks);
  std::atomic<size_t> total_costtime(0); // ✅ 改为线程安全累加
  for (size_t k = 0; k < nworks; ++k) {
    vthread[k] = std::thread([&]() {
      for (size_t j = 0; j < rounds; ++j) {
        auto begin1 = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < ntimes; i++) {
          P1 *p1 = new P1;
          delete p1;
          P2 *p2 = new P2;
          delete p2;
          P3 *p3 = new P3;
          delete p3;
          P4 *p4 = new P4;
          delete p4;
        }
        auto end1 = std::chrono::high_resolution_clock::now();
        total_costtime +=
            std::chrono::duration_cast<std::chrono::milliseconds>(end1 - begin1)
                .count();
      }
    });
  }
  for (auto &t : vthread) {
    t.join();
  }
  printf(
      "%lu个线程并发执行%lu轮次，每轮次malloc&free %lu次，总计花费：%lu ms\n",
      nworks, rounds, ntimes, total_costtime.load());
}

int main() {
  HashBucket::initMemoryPool(); // 使用内存池接口前一定要先调用该函数
  BenchmarkMemoryPool(100000, 1, 1000); // 测试内存池
  std::cout << "==============================================================="
               "======="
            << std::endl;
  std::cout << "==============================================================="
               "======="
            << std::endl;
  BenchmarkNew(100000, 1, 1000); // 测试 new delete

  return 0;
}
