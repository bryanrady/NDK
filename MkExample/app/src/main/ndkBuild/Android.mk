#源文件在的位置。宏函数 my-dir 返回Android.mk当前目录（包含 Android.mk 文件本身的目录）的路径。
LOCAL_PATH := $(call my-dir)
#输出Android.mk的当前路径
$(info "LOCAL_PATH:======== ${LOCAL_PATH}")

#预编译动态库的引入 (提前编译好的库)
#include $(CLEAR_VARS)
#LOCAL_MODULE := Test
#要进行编译的源文件
#LOCAL_SRC_FILES := libTest.so
#include $(PREBUILT_SHARED_LIBRARY)

#预编译静态库的引入 (提前编译好的库)
include $(CLEAR_VARS)
LOCAL_MODULE := Test
#源文件是一个动态库文件
LOCAL_SRC_FILES := libTest.a
include $(PREBUILT_STATIC_LIBRARY)

#引入其他makefile文件。CLEAR_VARS 变量指向特殊 GNU Makefile，可为您清除许多 LOCAL_XXX 变量
#不会清理 LOCAL_PATH 变量
include $(CLEAR_VARS)
#存储您要构建的模块的名称 每个模块名称必须唯一，且不含任何空格
#如果模块名称的开头已是 lib，则构建系统不会附加额外的前缀 lib；而是按原样采用模块名称，并添加 .so 扩展名。
LOCAL_MODULE := native-lib
#包含要构建到模块中的 C 和/或 C++ 源文件列表 以空格分开, 如果是换行的话 就是 \
LOCAL_SRC_FILES := native-lib.cpp abc.c \
ef.cpp
# 编译native-lib模块 需要链接 Test 模块
# Test模块是一个预编译库模块
#LOCAL_SHARED_LIBRARIES := Test
LOCAL_STATIC_LIBRARIES := Test
#构建共享动态库
include $(BUILD_SHARED_LIBRARY)