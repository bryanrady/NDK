#include <jni.h>
#include <string>
#include "librtmp/rtmp.h"
#include "safe_queue.h"
#include "macro.h"
#include "VideoChannel.h"

VideoChannel *videoChannel = 0;
SafeQueue<RTMPPacket*> packets;
bool isStart = 0;   //判断是否已经开始过直播
pthread_t pid_tcp;  //进行TCP连接的线程
int readyPushing = 0;
int start_time = 0;

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

void *start_tcp(void *args){
    char *url = static_cast<char *>(args);
    RTMP *rtmp = 0;
    //这里使用do while循环方便我们break
    do{
        //1.创建一个RTMP对象
        rtmp = RTMP_Alloc();
        if(!rtmp){
            LOGE("Rtmp创建失败!");
            break;
        }
        //2.初始化RTMP
        RTMP_Init(rtmp);
        //设置RTMP超时时间5s
        rtmp->Link.timeout = 5;
        //3.给RTMP设置url
        int ret = RTMP_SetupURL(rtmp,url);
        if(!ret){
            LOGE("Rtmp设置地址失败: %s",url);
            break;
        }
        //4.开启输出模式
        RTMP_EnableWrite(rtmp);
        //5.连接服务器,这里不用发包，传个0
        ret = RTMP_Connect(rtmp,0);
        if(!ret){
            LOGE("Rtmp连接服务器失败!");
            break;
        }
        //6.申请跟服务器创建流，然后才可以进行推送
        ret = RTMP_ConnectStream(rtmp,0);
        if(!ret){
            LOGE("Rtmp连接流失败!");
            break;
        }
        //记录一个开始的时间
        start_time = RTMP_GetTime();
        //表示我们可以进行推流了
        readyPushing = 1;
        //将packets设置为工作状态
        packets.setWork(1);
        RTMPPacket *packet = 0;
        while (isStart){
            ret = packets.pop(packet);
            //如果停止直播了，就退回
            if(!isStart){
                //如果停止了，还取出来了包，就进行释放
                if(ret){
                    releaseRTMPPackets(packet);
                }
                break;
            }
            //如果没有取出来就重新去取,这两种写法都是一个意思
//            if(!ret){
//                continue;
//            }
            if(!packet){
                continue;
            }
            //发送包之前注意：
            packet->m_nInfoField2 = rtmp->m_stream_id;
            //7.发送RTMPPacket包   1:队列 表示把包放到队列里，然后一个一个发送
            ret = RTMP_SendPacket(rtmp,packet,1);
            //这里发送出去以后就把packet释放掉
            releaseRTMPPackets(packet);
            if(!ret){
                LOGE("Rtmp发送包失败!");
                break;
            }
        }
        releaseRTMPPackets(packet);
    }while (0);

    //释放
    packets.setWork(0);
    packets.clear();
    if(rtmp){
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }
    DELETE(url);
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_rtmp_LivePusher_native_1start(JNIEnv *env, jobject instance, jstring _path) {
    if(isStart){    //如果开始了，就return
        return;
    }
    isStart = 1;
    const char *path = env->GetStringUTFChars(_path,0);
    //因为下面会把path给释放掉，这里我们重新进行拷贝
    char *url = new char[strlen(path)+1];
    strcpy(url,path);

    //启动线程进行Tcp连接
    pthread_create(&pid_tcp,0,start_tcp,url);

    env->ReleaseStringUTFChars(_path,path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_rtmp_LivePusher_native_1pushVideo(JNIEnv *env, jobject instance, jbyteArray _data) {
    if(!videoChannel){
        return;
    }
    if(!readyPushing){
        return;
    }
    jbyte *data = env->GetByteArrayElements(_data,0);
    //将数据交给videoChannel进行编码,jbyte实际上就是int8_t,所以直接传递jbyte  typedef int8_t   jbyte;    /* signed 8 bits */
    videoChannel->encodeData(data);
    env->ReleaseByteArrayElements(_data,data,0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_rtmp_LivePusher_native_1stop(JNIEnv *env, jobject instance) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_rtmp_LivePusher_native_1release(JNIEnv *env, jobject instance) {

}