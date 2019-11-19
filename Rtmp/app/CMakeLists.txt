
cmake_minimum_required(VERSION 3.4.1)

#引入指定目录(src/main/cpp/librtmp)下面的CMakeLists.txt
add_subdirectory(src/main/cpp/librtmp)

#指定头文件的搜索路径
include_directories(src/main/cpp/include)


add_library(
             native-lib

             SHARED

        src/main/cpp/native-lib.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/src/main/cpp/libx264/${ANDROID_ABI}")

target_link_libraries(
                       native-lib
                       rtmp
                       x264
                       log )