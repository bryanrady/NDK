
cmake_minimum_required(VERSION 3.4.1)

#这里添加<stdlib.h>是因为 atof函数现在变成了stdlib的函数，所以需要这样引用
include_directories(src/main/cpp/include <stdlib.h>)

# 创建一个变量 source_file 它的值就是src/main/cpp/ 所有的.cpp文件
file(GLOB source_file src/main/cpp/*.cpp)

add_library(
        native-lib

        SHARED

        ${source_file} )

#设置预编译ffmpeg静态库
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/src/main/cpp/jniLib/${ANDROID_ABI}")

# avfilter avformat avcodec avutil swresample swscale 这几个库的顺序不能打乱
# z 是因为FFmpeg依赖这个库
# android 是为了让native能够使用ANativeWindow RGB/YUV
# OpenSLES 使用这个来播放音频PCM
target_link_libraries(
        native-lib
        avfilter avformat avcodec avutil swresample swscale
        log
        z
        android
        OpenSLES)