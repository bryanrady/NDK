#这个文件的目的是生成一个rtmp静态库
cmake_minimum_required(VERSION 3.4.1)

#预编译宏 使用-D取消这个宏定义 NO_CRYPTO
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DNO_CRYPTO")

file(GLOB rtmp_source *.c)

add_library(
             rtmp

             STATIC

        ${rtmp_source})