#include <jni.h>
#include <string>
#include <android/log.h>
#include "safe_queue.h"
#define  LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"JNI",__VA_ARGS__);    //  __VA_ARGS__ 代表... 可变参数

extern "C"{
#include "include/libavutil/avutil.h"
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_bryanrady_ffmpeg_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    char *versionInfo = (char*)av_version_info();
    return env->NewStringUTF(versionInfo);
}
