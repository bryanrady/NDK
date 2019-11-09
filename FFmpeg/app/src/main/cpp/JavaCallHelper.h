//
// Created by Administrator on 2019/11/9.
//

#ifndef FFMPEG_JAVACALLHELPER_H
#define FFMPEG_JAVACALLHELPER_H

#include <jni.h>

/**
 * 反射执行java方法
 */
class JavaCallHelper{
public:
    //这里可能有跨线程操作，所以需要JavaVM,如果是在主线程使用，直接使用主线程传进来的env
    JavaCallHelper(JavaVM *vm,JNIEnv *env,jobject instance);
    ~JavaCallHelper();

    //thread参数是为了标记当前是否处于主线程，方便我们在c++进行回调
    void onError(int thread,int errorCode);
    void onPrepared(int thread);

private:
    JavaVM *vm;
    JNIEnv *env;
    jobject instance;
    jmethodID onErrorMethodId;
    jmethodID onPreparedMethodId;
};

#endif //FFMPEG_JAVACALLHELPER_H
