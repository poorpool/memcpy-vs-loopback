# CPU memcpy 对比 RNIC loopback message

fast_memcpy：

```
❯ ./build/fast_memcpy 
send 128.000 GiB in 15.127s, speed 8.462 GiB/s
```

memcpy：
```
❯ ./build/memcpy  
send 128.000 GiB in 24.610s, speed 5.201 GiB/s
```

loopback message：
```
❯ ./build/loopback
local lid 0 qp_num 34032 gid 00000000000000000000FFFFC0A8C835 gid_index 3
send 128.000 GiB in 15.116s, speed 8.468 GiB/s
```

loopback message（同时在 ib_write_bw）：

```
❯ ./build/loopback 
local lid 0 qp_num 34031 gid 00000000000000000000FFFFC0A8C835 gid_index 3
send 128.000 GiB in 22.795s, speed 5.615 GiB/s
```

结论：拆包合包的时候可以用 loopback message