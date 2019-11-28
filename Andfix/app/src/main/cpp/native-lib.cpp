#include <jni.h>
#include "art_method.h"
//https://github.com/alibaba/AndFix
//这个头文件是从阿里的andfix里面直接拷贝过来的,我们自己去找的话很复杂，这些是阿里的人员从dalvik中抽取出来的，所以直接用即可
#include "dalvik.h"

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_andfix_DexManager_replaceArt(JNIEnv *env, jobject thiz, jint sdk,
                                                jobject wrong_method, jobject right_method) {
    //找到art虚拟机对应的 ArtMethod 结构体 ArtMethod只需要申明的头文件就行，实现在art虚拟机里面
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
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_andfix_DexManager_replaceDalvik(JNIEnv *env, jobject thiz, jint sdk,
                                                   jobject wrong_method, jobject right_method) {
    //dalvik 虚拟机里面叫 Method 结构体 art虚拟机叫 ArtMethod
    //找到dalvik虚拟机对应的Method 结构体
    Method *wrongMethod = reinterpret_cast<Method *>(env->FromReflectedMethod(wrong_method));
    Method *rightMethod = reinterpret_cast<Method *>(env->FromReflectedMethod(right_method));

    //底层的Hook 直接打开一个so库，使用里面的方法
    //通过dlopen()函数得到一个动态链接库的操作句柄
    void *dvm_hand = dlopen("libdvm.so",RTLD_NOW);

}