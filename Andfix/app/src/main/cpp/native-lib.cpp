#include <jni.h>
#include "art_method.h"

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_andfix_DexManager_replace(JNIEnv *env, jobject thiz, jobject wrong_method,
                                             jobject right_method) {
    //ArtMethod只需要申明的头文件就行，实现在art虚拟机里面
    art::mirror::ArtMethod *wrongMethod = reinterpret_cast<art::mirror::ArtMethod *>(env->FromReflectedMethod(wrong_method));
    art::mirror::ArtMethod *rightMethod = reinterpret_cast<art::mirror::ArtMethod *>(env->FromReflectedMethod(right_method));

    //不是方法直接替换
    //wrongMethod = rightMethod;

    //而是ArtMethod里面的成员变量进行替换
    wrongMethod->declaring_class_ = rightMethod->declaring_class_;
    wrongMethod->dex_cache_resolved_methods_ = rightMethod->dex_cache_resolved_methods_;
    wrongMethod->access_flags_ = rightMethod->access_flags_;
    wrongMethod->dex_cache_resolved_types_ = rightMethod->dex_cache_resolved_types_;
    wrongMethod->dex_code_item_offset_ = rightMethod->dex_code_item_offset_;
    wrongMethod->dex_method_index_ = rightMethod->dex_method_index_;
    wrongMethod->method_index_ = rightMethod->method_index_;
    env->FindClass()
}