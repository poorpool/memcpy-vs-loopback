cmake_minimum_required(VERSION 3.0.0)
project(memcpy_vs_loopback VERSION 0.1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Release")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -g -ggdb -fno-omit-frame-pointer -fno-inline-functions -march=native")
# SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -g -ggdb -fsanitize=address -static-libsan")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -march=native")

add_executable(loopback loopback.cc rdma.cc)
target_link_libraries(loopback ibverbs)

add_executable(memcpy memcpy.cc)
add_executable(fast_memcpy fast_memcpy.cc)