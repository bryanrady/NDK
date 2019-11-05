//
// Created by Administrator on 2019/11/4.
//

#include <jni.h>
#include <android/log.h>
#include "include/test1.h"

//cpp文件引入c文件的时候，需要加入这个
extern "C"{
    extern int test();
}

extern "C"
void Java_com_bryanrady_mkexample_MainActivity_nativeTest(JNIEnv *env, jobject instance){
    test();
    include_test();
}