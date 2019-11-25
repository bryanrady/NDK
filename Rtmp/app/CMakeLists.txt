
cmake_minimum_required(VERSION 3.4.1)

#引入指定目录(src/main/cpp/librtmp)下面的CMakeLists.txt
add_subdirectory(src/main/cpp/librtmp)

#指定头文件的搜索路径
include_directories(src/main/cpp/include)

file(GLOB source_file src/main/cpp/*.cpp *.c)

add_library(
             native-lib

             SHARED

             ${source_file})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/src/main/cpp/libs/${ANDROID_ABI}")

target_link_libraries(
                       native-lib
                       rtmp
                       x264
                       faac
                       log)