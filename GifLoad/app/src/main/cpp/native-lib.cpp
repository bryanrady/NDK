#include <jni.h>
#include <string>
#include <gif_lib.h>

extern "C" JNIEXPORT jlong JNICALL
Java_com_bryanrady_gifload_GifHandler_native_1loadGif(JNIEnv *env,jobject instance,jstring path_) {
    //gif图片路径
    const char *path = env->GetStringUTFChars(path_,0);


    env->ReleaseStringUTFChars(path_,path);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_bryanrady_gifload_GifHandler_native_1getWidth(JNIEnv *env, jobject instance, jlong ndk_gif) {

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_bryanrady_gifload_GifHandler_native_1getHeight(JNIEnv *env, jobject instance, jlong ndk_gif) {

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_bryanrady_gifload_GifHandler_native_1updateFrame(JNIEnv *env, jobject instance, jlong ndk_gif,
                                                  jobject bitmap) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_gifload_GifHandler_native_1release(JNIEnv *env, jobject instance, jlong ndk_gif) {

}