//
// Created by wangqb on 2019/11/9.
//

#include "BaseChannel.h"
#include "macro.h"

BaseChannel::BaseChannel(int stream_id,AVCodecContext *codecContext,AVRational time_base,JavaCallHelper *callHelper){
    this->stream_id = stream_id;
    this->codecContext = codecContext;
    this->time_base = time_base;
    this->callHelper = callHelper;

    isPlaying = 0;
    isPause = 0;
    pthread_mutex_init(&pauseMutex, 0);
    pthread_cond_init(&pauseCond,0);

    packets.setReleaseCallback(BaseChannel::releaseAvPacket);
    //packets.setReleaseCallback2(BaseChannel::releaseAvPacket2);
    frames.setReleaseCallback(BaseChannel::releaseAVFrame);
}

BaseChannel::~BaseChannel() {
    if(codecContext){
        avcodec_close(codecContext);
        avcodec_free_context(&codecContext);
        codecContext = 0;
    }
    clearQueue();
    pthread_mutex_destroy(&pauseMutex);
    pthread_cond_destroy(&pauseCond);
}

void BaseChannel::startWork() {
    packets.setWork(1);
    frames.setWork(1);
}

void BaseChannel::stopWork() {
    packets.setWork(0);
    frames.setWork(0);
}

void BaseChannel::clearQueue() {
    packets.clear();
    frames.clear();
}

void BaseChannel::releaseAvPacket(AVPacket **packet) {
    if (packet){
        //av_packet_free和av_packet_alloc()呈对应关系
        //存放音频包或者视频包的队列,av_packet_alloc()内部获取的AVPacket *包所占用的内存是堆内存
        // ，所以我们在调用队列的clear要手动的来释放
        av_packet_free(packet);
        *packet = 0;
    }
}

void BaseChannel::releaseAvPacket2(AVPacket *&packet) {
    if (packet){
        //av_packet_free和av_packet_alloc()呈对应关系
        av_packet_free(&packet);
        packet = 0;
    }
}

void BaseChannel::releaseAVFrame(AVFrame **avFrame) {
    if(avFrame){
        av_frame_free(avFrame);
        *avFrame = 0;
    }
}

void BaseChannel::pause() {
    pthread_mutex_lock(&pauseMutex);
    if(isPause == 0){
        isPause = 1;
    }else{
        isPause = 0;
    }
    LOGE("isPause11 %d",isPause);
    if(isPause==0){
        pthread_cond_signal(&pauseCond);
    }
    pthread_mutex_unlock(&pauseMutex);
}