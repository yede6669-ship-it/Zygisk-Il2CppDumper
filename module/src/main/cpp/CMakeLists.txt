cmake_minimum_required(VERSION 3.18.1)

project(il2cppdumper)

set(CMAKE_CXX_STANDARD 20)

# 确保 JNI 符号可见，不要隐藏
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default")

include_directories(
    .
    xdl/include
)

aux_source_directory(xdl xdl-src)

# 编译目标
add_library(${MODULE_NAME} SHARED
        main.cpp
        hack.cpp
        il2cpp_dump.cpp
        ${xdl-src})

# 链接日志库
target_link_libraries(${MODULE_NAME} log)
