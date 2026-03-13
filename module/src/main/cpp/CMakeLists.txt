cmake_minimum_required(VERSION 3.18.1)

# 定义项目名称
project(il2cppdumper)

# 如果 Gradle 没有传入 MODULE_NAME，则手动设置一个默认值
if(NOT MODULE_NAME)
    set(MODULE_NAME "il2cppdumper")
endif()

set(CMAKE_CXX_STANDARD 20)

# 核心：确保 JNI 导出函数在外部可见
# 同时增加优化参数，提高 Dump 效率
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default -O2 -Wall")

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/xdl/include
)

# 自动搜寻 xdl 相关源码
aux_source_directory(xdl xdl-src)

# 编译目标
# 包含主入口、IL2CPP 脱壳逻辑以及 xdl 链接器辅助工具
add_library(${MODULE_NAME} SHARED
        main.cpp
        il2cpp_dump.cpp
        ${xdl-src}
)

# 链接 Android 日志库和其他系统库
# log: 用于 __android_log_print
# dl:  用于 dlopen/dlsym 操作
target_link_libraries(${MODULE_NAME} 
    log 
    dl
)
