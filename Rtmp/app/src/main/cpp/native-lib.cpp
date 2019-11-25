#include <jni.h>
#include <string>
#include "librtmp/rtmp.h"
#include "safe_queue.h"
#include "macro.h"
#include "VideoChannel.h"
#include "AudioChannel.h"

VideoChannel *videoChannel = 0;
AudioChannel *audioChannel = 0;

SafeQueue<RTMPPacket*> packets;
pthread_t pid_tcp;  //进行TCP连接的线程
bool isStart = 0;   //判断是否已经开始过直播
bool readyPushing = 0;  //判断是否可以开始进行推流
uint32_t start_time = 0;

void releaseRTMPPackets(RTMPPacket*& packet){
    if (packet) {
        RTMPPacket_Free(packet);
        DELETE(packet);
    }
}

/**
 * RTMP包组装完成后，就添加时间戳然后push到队列中
 */
void rtmpPacketCompleted(RTMPPacket *packet){
    if(packet){
        //设置时间戳
       packet->m_nTimeStamp = RTMP_GetTime() - start_time;
       //添加到队列中去
       packets.push(packet);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_bryanrady_rtmp_LivePusher_native_1init(JNIEnv* env,jobject instance) {
    //准备一个Video编码器的工具类，通过这个工具来进行编码
    videoChannel = new VideoChannel;
    //这里设置了回调，当RTMP包组装完成后就会回调到rtmpPacketCompleted这个方法中
    videoChannel->setVideoCallback(rtmpPacketCompleted);

    //准备一个Audio编码器，通过AudioChannel来进行编码
    audioChannel = new AudioChannel;
    audioChannel->setAudioCallback(rtmpPacketCompleted);

    //设置回调
    packets.setReleaseCallback(releaseRTMPPackets);

    //接下来要做的就是准备一个队列,打包好的数据 放入队列，在线程中统一的取出数据再发送给服务器
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

        //第2个Bug：超时时间不生效，对连接服务器超时5s并没有效果，它会等到默认的30s后才告诉我们连接失败。
        //解决办法: 在rtmp.c内部的RTMP_Connect0()方法中进行修改。
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
        if (!ret) {
            LOGE("Rtmp连接地址失败:%s", url);
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

        //将acc解码序列包添加到队列中的第一个，这里调用一次保证第一个数据是 aac解码数据包，然后后面发送的才是音频裸数据
        rtmpPacketCompleted(audioChannel->getAudioTag());

        RTMPPacket *packet = 0;
        while (readyPushing){
            ret = packets.pop(packet);
            //如果停止直播了，就退回
            if(!readyPushing){
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

            //rtmp的第一个Bug：
            //发送rtmp包 1：队列  会调用 WriteN
            // 但是当发生意外断网？发送包失败，rtmpdump 内部的 WriteN 会调用RTMP_Close
            // RTMP_Close 会调用SendFCUnpublish 会调用SendFCUnpublish又会调用 RTMP_SendPacket
            // RTMP_SendPacket  最后又会调用到 RTMP_Close  这样就会造成一直递归调用。
            // 解决办法: 将rtmp.c 里面WriteN方法的 Rtmp_Close注释掉

            //那么我们为什么不注释掉SendFCUnpublish?而要注释掉RTMP_Close ？
            //(1)首先我们不需要rtmp内部帮我们调用RTMP_Close，我们自己外部就会调用到RTMP_Close
            //(2)如果我们没发生断网是正常向服务器推流，当我们要停止直播的时候，SendFCUnpublish 就会通知服务器客户端不想推流了。

            //7.发送RTMPPacket包   1:队列 表示把包放到队列里，然后一个一个发送
            ret = RTMP_SendPacket(rtmp, packet, 1);
            //这里发送出去以后就把packet释放掉
            releaseRTMPPackets(packet);
            if(!ret){
                LOGE("Rtmp发送包失败!");
                break;
            }
            LOGE("Rtmp发送包成功!");
        }
        releaseRTMPPackets(packet);
    }while (0);
    //释放
    isStart = 0;
    readyPushing = 0;
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
    pthread_create(&pid_tcp, 0, start_tcp, url);

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
    jbyte *data = env->GetByteArrayElements(_data, 0);
    //将数据交给videoChannel进行编码,jbyte实际上就是int8_t,所以直接传递jbyte  typedef int8_t   jbyte;    /* signed 8 bits */
    videoChannel->encodeData(data);
    env->ReleaseByteArrayElements(_data, data, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_rtmp_LivePusher_native_1stop(JNIEnv *env, jobject instance) {
    readyPushing = 0;
    packets.setWork(0);
    pthread_join(pid_tcp, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_rtmp_LivePusher_native_1release(JNIEnv *env, jobject instance) {
    //release 和 init 进行对应 上面现在只new 出来一个 videoChannel
    DELETE(videoChannel);
    DELETE(audioChannel);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_rtmp_LivePusher_native_1setAudioEncInfo(JNIEnv *env, jobject instance,
                                                           jint sampleRateInHz, jint channels) {
    if(audioChannel){
        audioChannel->setAudioEncInfo(sampleRateInHz, channels);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_bryanrady_rtmp_LivePusher_getInputSamples(JNIEnv *env, jobject instance) {
    if(audioChannel){
        return audioChannel->getInputSamples();
    }
    return -1;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_rtmp_LivePusher_native_1pushAudio(JNIEnv *env, jobject instance, jbyteArray _data) {
    if(!audioChannel){
        return;
    }
    if(!readyPushing){
        return;
    }
    jbyte* data = env->GetByteArrayElements(_data, 0);
    audioChannel->encodeData(data);
    env->ReleaseByteArrayElements(_data, data, 0);
}