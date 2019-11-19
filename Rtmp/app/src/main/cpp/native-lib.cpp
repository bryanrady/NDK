#include <jni.h>
#include <string>
#include "librtmp/rtmp.h"
#include "safe_queue.h"
#include "macro.h"
#include "VideoChannel.h"

VideoChannel *videoChannel = 0;
SafeQueue<RTMPPacket*> packets;

void releaseRTMPPackets(RTMPPacket*& packet){
    if (packet) {
        RTMPPacket_Free(packet);
        delete packet;
        packet = 0;
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_bryanrady_rtmp_LivePusher_native_1init(JNIEnv* env,jobject instance) {
    //准备一个Video编码器的工具类，通过这个工具来进行编码
    videoChannel = new VideoChannel;
    //准备一个队列,打包好的数据 放入队列，在线程中统一的取出数据再发送给服务器
    packets.setReleaseCallback(releaseRTMPPackets);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_rtmp_LivePusher_native_1setVideoEncInfo(JNIEnv *env, jobject instance, jint width,
                                                           jint height, jint fps, jint bitrate) {
    if(videoChannel){
        videoChannel->setVideoEncInfo(width,height,fps,bitrate);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_rtmp_LivePusher_native_1start(JNIEnv *env, jobject instance, jstring path) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_rtmp_LivePusher_native_1pushVideo(JNIEnv *env, jobject instance, jbyteArray data) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_rtmp_LivePusher_native_1stop(JNIEnv *env, jobject instance) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_rtmp_LivePusher_native_1release(JNIEnv *env, jobject instance) {

}