#include <jni.h>
#include <string>
#include "DNFFmpeg.h"
#include "JavaCallHelper.h"
#include <android/native_window_jni.h>
#include "macro.h"

extern "C"{
#include <libavutil/avutil.h>
}

DNFFmpeg *dnfFmpeg = 0;
JavaVM *_vm = 0;
ANativeWindow *nativeWindow = 0;
//通过静态的方式进行互斥锁的初始化，只要我们要来操作nativeWindow,我们就会一直使用它，所以可以不用释放
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int JNI_OnLoad(JavaVM* vm, void* reserved){
    _vm = vm;
    //这里返回1.2、1.4、1.6都没有太大的影响
    return JNI_VERSION_1_6;
}

//渲染播放 就是将RGBA数据渲染到nativewindow上
void render(uint8_t *data,int line_size,int width,int height){
    pthread_mutex_lock(&mutex);
    if(nativeWindow == NULL){
        pthread_mutex_unlock(&mutex);
        return;
    }
    //设置窗口属性
    ANativeWindow_setBuffersGeometry(nativeWindow,width,height,WINDOW_FORMAT_RGBA_8888);
    //这里不知道为什么ANativeWindow_Buffer不能用指针,用指针就会崩溃
    ANativeWindow_Buffer nativeWindow_Buffer;
    if(ANativeWindow_lock(nativeWindow,&nativeWindow_Buffer,0) != 0){
        ANativeWindow_release(nativeWindow);
        nativeWindow = 0;
        pthread_mutex_unlock(&mutex);
        return;
    }
    //填充RGBA数据给new_data
    uint8_t *dst_data = static_cast<uint8_t *>(nativeWindow_Buffer.bits);
    //stride 一行有多少个RGBA数据 RGBA * 4 表示有多少个字节
    int dst_linesize = nativeWindow_Buffer.stride * 4;
    //一行一行的进行拷贝,这里还要注意个问题，我们在c++线程里面进行拷贝数据的时候，java线程调用setSurface可能会把老的nativeWindow进行释放
    //所以可能会导致崩溃，所以要进行同步
    for (size_t i = 0; i < nativeWindow_Buffer.height; ++i) {
        //将data里面的数据拷贝到dst_data中，每次拷贝dst_linesize个字节
        memcpy(dst_data + i * dst_linesize,data + i * line_size, dst_linesize);
    }
    ANativeWindow_unlockAndPost(nativeWindow);

    pthread_mutex_unlock(&mutex);
};

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_ffmpeg_DNPlayer_native_1prepare(JNIEnv *env, jobject instance, jstring data_source) {
    //获取要播放的视频地址
    const char *dataSource = env->GetStringUTFChars(data_source,0);
    JavaCallHelper *callHelper = new JavaCallHelper(_vm,env,instance);
    dnfFmpeg = new DNFFmpeg(dataSource,callHelper);
    //回调render这个函数
    dnfFmpeg->setRenderFrameCallback(render);
    dnfFmpeg->prepare();
    env->ReleaseStringUTFChars(data_source,dataSource);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_ffmpeg_DNPlayer_native_1start(JNIEnv *env, jobject instance) {
    dnfFmpeg->start();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_ffmpeg_DNPlayer_native_1setSurface(JNIEnv *env, jobject instance, jobject surface) {
    pthread_mutex_lock(&mutex);
    //这里要注意随时都会把新的surface传递进来,所以我们要判断一下将老的nativeWindow进行释放
    if(nativeWindow != NULL){
        ANativeWindow_release(nativeWindow);
        nativeWindow = 0;
    }
    //将java的suface变成native的窗口
    nativeWindow = ANativeWindow_fromSurface(env,surface);
    pthread_mutex_unlock(&mutex);
}