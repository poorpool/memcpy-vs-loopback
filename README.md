# CPU memcpy 对比 RNIC loopback message

memcpy：

```
send 128.000 GiB in 16.608s, speed 7.707 GiB/s
```

fast_memcpy：

```
send 128.000 GiB in 15.341s, speed 8.343 GiB/s
```

loopback message：

```
local lid 0 qp_num 11598 gid 00000000000000000000FFFFC0A8C835 gid_index 3
send 128.000 GiB in 15.091s, speed 8.482 GiB/s
```

loopback message（同时在 ib_write_bw）：

```
local lid 0 qp_num 11597 gid 00000000000000000000FFFFC0A8C835 gid_index 3
send 128.000 GiB in 22.877s, speed 5.595 GiB/s
```

fast_memcpy（同时在 ib_write_bw）：
```
send 128.000 GiB in 17.118s, speed 7.477 GiB/s
```

结论：拆包合包的时候 memcpy 线程更稳定
