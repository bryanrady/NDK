//
// Created by wangqb on 2019/11/9.
//

#include "BaseChannel.h"

BaseChannel::BaseChannel(int stream_id) {
    this->stream_id = stream_id;
}

BaseChannel::~BaseChannel() {
    packets.setReleaseCallback(BaseChannel::releaseAvPacket);
    //packets.setReleaseCallback2(BaseChannel::releaseAvPacket2);
    packets.clear();
}

static void BaseChannel::releaseAvPacket(AVPacket **packet) {
    if (packet != NULL){
        //av_packet_free和av_packet_alloc()呈对应关系
        //存放音频包或者视频包的队列,av_packet_alloc()内部获取的AVPacket *包所占用的内存是堆内存
        // ，所以我们在调用队列的clear要手动的来释放
        av_packet_free(packet);
        *packet = 0;
    }
}

static void BaseChannel::releaseAvPacket2(AVPacket *&packet) {
    if (packet != NULL){
        //av_packet_free和av_packet_alloc()呈对应关系
        av_packet_free(&packet);
        packet = 0;
    }
}