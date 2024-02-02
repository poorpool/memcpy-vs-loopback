#include "rdma.h"
#include <chrono>
#include <cstring>
#include <iostream>
#include <malloc.h>
#include <string>

using std::string;

int main() {
  char *buf1 =
      reinterpret_cast<char *>(memalign(4096, kTranferSize * kCntPerTranfer));
  char *buf2 =
      reinterpret_cast<char *>(memalign(4096, kTranferSize * kCntPerTranfer));
  auto start_time = std::chrono::high_resolution_clock::now();
  for (int task = 0; task < kTaskCnt; task++) {
    int sended = 0;
    while (sended < kCntPerTranfer) {
      for (int i = sended; i - sended + 1 <= 16 && i < kCntPerTranfer; i++) {
        memcpy(buf2 + i * kTranferSize, buf1 + i * kTranferSize, kTranferSize);
      }
      sended = std::min(sended + 16, kCntPerTranfer);
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