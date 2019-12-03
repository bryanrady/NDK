cmake_minimum_required(VERSION 3.4.1)

#引入指定目录(src/main/cpp/libgif)下面的CMakeLists.txt
add_subdirectory(src/main/cpp/libgif)

add_library(
        native-lib

        SHARED

        src/main/cpp/native-lib.cpp)


target_link_libraries(
        native-lib

        gif

        jnigraphics

        log)