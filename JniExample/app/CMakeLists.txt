# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/native-lib.cpp )

#设置一个变量
# CMAKE_CXX_FLAGS c++的参数 会传给编译器
# CMAKE_C_FLAGS c的参数 会传给编译器
# CMAKE_CXX_FLAGS=--sysroot=XX
# 重新定义  CMAKE_CXX_FLAGS 变量  +
# CMAKE_SOURCE_DIR 的值是当前这个文件的地址
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}")

#这种方式也可以
#add_library(Test SHARED IMPORTED)
#set_target_properties(Test PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi-v7a/libTest.so)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
#这句话的意思是 去ndk目录下 D:\androidSdk\android\sdk\ndk-bundle\platforms\android-21\arch-arm\usr\lib ，随便一个版本
#这些预测的目录的log.so库，然后把找到的log库绝对路径赋值给log-lib,
find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.
#然后这里把log.so的绝对路径写到 ${log-lib} 这里面，所以我们就能在native-lib中使用log.so的函数功能了;
# ${log-lib} 也可以简写 直接写一个 log 就可以
target_link_libraries( # Specifies the target library.
                       native-lib
                       Test
                       # Links the target library to the log library
                       # included in the NDK
                       ${log-lib} )