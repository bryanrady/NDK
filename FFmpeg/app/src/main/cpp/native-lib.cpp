#include <jni.h>
#include <string>
#include "safe_queue.h"
#include "DNFFmpeg.h"
#include "JavaCallHelper.h"

extern "C"{
#include "include/libavutil/avutil.h"
}

DNFFmpeg *dnfFmpeg = 0;
JavaVM *_vm = 0;

int JNI_OnLoad(JavaVM* vm, void* reserved){
    _vm = vm;
    //这里返回1.2、1.4、1.6都没有太大的影响
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_ffmpeg_DNPlayer_native_1prepare(JNIEnv *env, jobject instance, jstring data_source) {
    // TODO: implement native_prepare()
    //获取要播放的视频地址
    const char *dataSource = env->GetStringUTFChars(data_source,0);
    JavaCallHelper *callHelper = new JavaCallHelper(_vm,env,instance);
    dnfFmpeg = new DNFFmpeg(dataSource,callHelper);
    dnfFmpeg->prepare();

    env->ReleaseStringUTFChars(data_source,dataSource);
}