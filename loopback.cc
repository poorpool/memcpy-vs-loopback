#include "rdma.h"
#include <chrono>
#include <infiniband/verbs.h>
#include <iostream>
#include <malloc.h>
#include <string>

using std::cerr;
using std::endl;
using std::string;

struct ServerContext {
  int link_type; // IBV_LINK_LAYER_XX
  RdmaDeviceInfo dev_info;
  char *buf1;
  char *buf2;
  ibv_mr *mr1;
  ibv_mr *mr2;
  ibv_cq *cq;
  ibv_qp *qp;

  void BuildRdmaEnvironment(const string &dev_name) {
    // 1. dev_info and pd
    link_type = IBV_LINK_LAYER_UNSPECIFIED;
    auto dev_infos = RdmaGetRdmaDeviceInfoByNames({dev_name}, link_type);
    if (dev_infos.size() != 1 || link_type == IBV_LINK_LAYER_UNSPECIFIED) {
      cerr << "query " << dev_name << "failed" << endl;
      exit(0);
    }
    dev_info = dev_infos[0];

    // 2. mr and buffer
    buf1 =
        reinterpret_cast<char *>(memalign(4096, kTranferSize * kCntPerTranfer));
    buf2 =
        reinterpret_cast<char *>(memalign(4096, kTranferSize * kCntPerTranfer));
    mr1 = ibv_reg_mr(dev_info.pd, buf1, kTranferSize * kCntPerTranfer,
                     IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
                         IBV_ACCESS_REMOTE_READ);
    if (mr1 == nullptr) {
      cerr << "register mr1 failed" << endl;
      exit(0);
    }
    mr2 = ibv_reg_mr(dev_info.pd, buf2, kTranferSize * kCntPerTranfer,
                     IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
                         IBV_ACCESS_REMOTE_READ);
    if (mr2 == nullptr) {
      cerr << "register mr1 failed" << endl;
      exit(0);
    }

    // 3. create cq
    cq = dev_info.CreateCq(kRdmaQueueSize * 2);
    if (cq == nullptr) {
      cerr << "create cq failed" << endl;
      exit(0);
    }

    // 4. create loopback qp
    qp = RdmaCreateQp(dev_info.pd, cq, cq, kRdmaQueueSize, IBV_QPT_RC);
    if (qp == nullptr) {
      cerr << "create qp failed" << endl;
      exit(0);
    }
    RdmaQpExchangeInfo local_info;
    local_info.lid = dev_info.port_attr.lid;
    local_info.qpNum = qp->qp_num;
    ibv_query_gid(dev_info.ctx, kRdmaDefaultPort, kGidIndex, &local_info.gid);
    local_info.gid_index = kGidIndex;
    printf("local lid %d qp_num %d gid %s gid_index %d\n", local_info.lid,
           local_info.qpNum, RdmaGid2Str(local_info.gid).c_str(),
           local_info.gid_index);

    RdmaModifyQp2Rts(qp, local_info, local_info);
  }

  void DestroyRdmaEnvironment() {
    if (qp != nullptr) {
      ibv_destroy_qp(qp);
      qp = nullptr;
    }

    ibv_destroy_cq(cq);
    ibv_dereg_mr(mr1);
    free(buf1);
    ibv_dereg_mr(mr2);
    free(buf2);
    ibv_dealloc_pd(dev_info.pd);
    ibv_close_device(dev_info.ctx);
  }
} s_ctx;

int main() {
  s_ctx.BuildRdmaEnvironment("mlx5_0");

  ibv_wc wc[kPollCqSize];
  auto start_time = std::chrono::high_resolution_clock::now();
  for (int task = 0; task < kTaskCnt; task++) {
    int remain = kCntPerTranfer;
    int sended = 0;
    while (remain > 0) {
      int n = ibv_poll_cq(s_ctx.cq, kPollCqSize, wc);
      remain -= n;
      for (int i = 0; i < n; i++) {
        if (wc[i].status != IBV_WC_SUCCESS) {
          cerr << "write failed" << endl;
        }
        if (wc[i].opcode != IBV_WC_RDMA_WRITE) {
          cerr << "unrecognized opcode" << endl;
        }
      }
      for (int i = sended; i - sended + 1 <= 16 && i < kCntPerTranfer; i++) {
        RdmaPostWrite(kTranferSize, s_ctx.mr1->lkey, i, s_ctx.qp,
                      s_ctx.buf1 + i * kTranferSize, s_ctx.mr2->rkey,
                      reinterpret_cast<uint64_t>(s_ctx.buf2) +
                          i * kTranferSize);
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
  s_ctx.DestroyRdmaEnvironment();
  return 0;
}