//
// Created by Administrator on 2019/11/4.
//

#include <jni.h>
#include <android/log.h>
#define  LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"JNI",__VA_ARGS__);    //  __VA_ARGS__ 代表... 可变参数

//cpp文件引入c文件的时候，需要加入这个
extern "C"{
    extern int test();
}

extern "C"
void Java_com_bryanrady_mkexample_MainActivity_nativeTest(JNIEnv *env, jobject instance){
    LOGE("动态库中的test()方法返回值:",test());
}