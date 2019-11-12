//
// Created by Administrator on 2019/11/9.
//

#include "JavaCallHelper.h"
#include "macro.h"

JavaCallHelper::JavaCallHelper(JavaVM *vm,JNIEnv *env,jobject instance) {
    this->vm = vm;
    this->env = env;
    //这里的instance不能直接相等
    //this->instance = instance;
    //如果涉及到跨方法/跨线程使用jobject,就要使用全局引用
    this->instance = env->NewGlobalRef(instance);
    jclass dnPlayerCls = env->GetObjectClass(instance);
    this->onErrorMethodId = env->GetMethodID(dnPlayerCls,"onError","(I)V");
    this->onPreparedMethodId = env->GetMethodID(dnPlayerCls,"onPrepared","()V");
}

JavaCallHelper::~JavaCallHelper() {
    env->DeleteGlobalRef(instance);
}

void JavaCallHelper::onError(int thread, int errorCode) {
    if(thread == THREAD_MAIN){
        //可以直接使用env
        env->CallVoidMethod(instance,onErrorMethodId,errorCode);
    }else{
        //子线程中需要借助JavaVM获得属于当前线程的JNIEnv
        JNIEnv *env = 0;
        //把当前native线程附加到java 虚拟机中
        jint ret = vm->AttachCurrentThread(&env,0);
        if (ret == JNI_OK){
            env->CallVoidMethod(instance,onErrorMethodId,errorCode);
            LOGE("C++反射调用Java onError");
        }
        vm->DetachCurrentThread();
    }
}

void JavaCallHelper::onPrepared(int thread) {
    if(thread == THREAD_MAIN){
        //可以直接使用env
        env->CallVoidMethod(instance,onPreparedMethodId);
    }else{
        //子线程中需要借助JavaVM获得属于当前线程的JNIEnv
        JNIEnv *env  = 0;
        //把当前native线程附加到java 虚拟机中
        jint ret = vm->AttachCurrentThread(&env,0);
        if (ret == JNI_OK){
            env->CallVoidMethod(instance,onPreparedMethodId);
            LOGE("C++反射调用Java onPrepared");
        }
        vm->DetachCurrentThread();

    }

}