#include "rdma.h"
#include <chrono>
#include <cstring>
#include <iostream>
#include <malloc.h>
#include <string>
#include "fast_memcpy.h"

using std::string;

int main() {
  // 换一种分配方式，降低缓存的影响
  char *buf1 =
      reinterpret_cast<char *>(memalign(4096, kTranferSize * kTaskCnt));
  char *buf2 =
      reinterpret_cast<char *>(memalign(4096, kTranferSize * kTaskCnt));
  auto start_time = std::chrono::high_resolution_clock::now();
  for (int task = 0; task < kCntPerTranfer; task++) {
    int sended = 0;
    while (sended < kTaskCnt) {
      for (int i = sended; i - sended + 1 <= 16 && i < kTaskCnt; i++) {
        memcpy_fast(buf2 + i * kTranferSize, buf1 + i * kTranferSize, kTranferSize);
      }
      sended = std::min(sended + 16, kTaskCnt);
    }
  }
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration_in_s = std::chrono::duration_cast<std::chrono::microseconds>(
                           end_time - start_time)
                           .count() /
                       1000000.0;
  double gibs =
      1.0 * kTranferSize * kCntPerTranfer * kTaskCnt / 1024.0 / 1024.0 / 1024.0;
  printf("send %.3f GiB in %.3fs, speed %.3f GiB/s\n", gibs, duration_in_s,
         gibs / duration_in_s);
  free(buf1);
  free(buf2);
  return 0;
}