# CPU memcpy 对比 RNIC loopback message

memcpy：
```
❯ ./build/memcpy
send 128.000 GiB in 3.314s, speed 38.626 GiB/s
```

loopback message：
```
❯ ./build/loopback 
local lid 0 qp_num 34028 gid 00000000000000000000FFFFC0A8C835 gid_index 3
send 128.000 GiB in 15.115s, speed 8.469 GiB/s
```

memcpy胜利！