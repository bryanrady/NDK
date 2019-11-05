#cmake脚本CMakeLists.txt文件开头第一句话 指定cmake的最小版本
cmake_minimum_required(VERSION 3.4.1)

#写了这句话后 就可以直接引用test1.h头文件
include_directories(src/main/cmake/include)

#如果要引入其他目录的CMakeLists.txt文件，XX是绝对路径
#add_subdirectory(XX)

#如果源文件很多的情况，可以这样来指定
#source是变量名 就是把这些目录下的以.c和.cpp结尾的文件添加到source变量中来
file(GLOB source src/main/cmake/*.c src/main/cmake/*.cpp)
add_library(
            #第一个参数是最终要生成的库的名字 libnative-lib.so
            native-lib

            #第2个参数是要生成的库是动态库 还是静态库 STATIC
            SHARED

            #第3个参数是要进行编译的源文件
            ${source})

#----------------------------------引入静态库start-----------------------------------------------
#引入静态库
#add_library(
#            #这个参数表示引入的库的名字 libTest.a
#            Test
#
#            #引入静态库
#            STATIC
#
#            #表示我们这一个静态库是以导入的形式添加进来(也就是预编译静态库)
#            IMPORTED)
#
##怎么导入？就要设置导入的路径
## CMAKE_SOURCE_DIR: 当前cmakelists.txt的路径 （cmake工具内置的）
## android cmake 内置的 ANDROID_ABI :  当前需要编译的cpu架构
## ANDROID_ABI 如果需要编译出多个平台的 so的时候，就可以使用这个变量来进行判断和引入
##可以用message来进行输出
##message("ANDROID_ABI : ${ANDROID_ABI}")
#set_target_properties(
#            #设置导入目标是Test模块
#            Test
#
#            #设置属性
#            PROPERTIES
#
#            #设置Test的 导入路径 属性
#            IMPORTED_LOCATION
#
#            #设置导入的库libTest.a的绝对路径,${CMAKE_SOURCE_DIR}变量就是CMakeLists.txt当前路径,这个变量相当于mk的LOCAL_PATH
#            ${CMAKE_SOURCE_DIR}/src/main/cmake/${ANDROID_ABI}/libTest.a)

#---------------------------------- 引入静态库end-----------------------------------------------

#---------------------------------- 引入动态库start-----------------------------------------------
#java.lang.UnsatisfiedLinkError: dlopen failed: library "../../../../src/main/jniLibs/armeabi-v7a/libTest.so" not found
#运行到6.0以上会报错，原因和Android.mk是一模一样的，所以通过这种方式引入动态库在6.0以上有问题
#add_library(Test2 SHARED IMPORTED)
#set_target_properties(Test2 PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libTest.so)

#但是cmake也还有其他办法引入动态库,让它没问题
# c: CMAKE_C_FLAGS
# set 方法 定义一个变量
#CMAKE_C_FLAGS = "${CMAKE_C_FLAGS} XXXX"
# -L: 库的查找路径 libTest.so
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}")

#---------------------------------- 引入动态库end-----------------------------------------------

#这些预测的目录的log.so库，然后把找到的log库绝对路径赋值给log-lib,
#可以不写 直接在 target_link_libraries 写上log
#find_library( # Sets the name of the path variable.
#        log-lib
#
#        # Specifies the name of the NDK library that
#        # you want CMake to locate.
#        log )

#需要链接的库模块
target_link_libraries(
                      native-lib
                      #Test
                      #Test2
                      Test
                      #${log-lib}
                      log)










