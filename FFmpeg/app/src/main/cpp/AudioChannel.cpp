//
// Created by Administrator on 2019/11/9.
//

#include "AudioChannel.h"

AudioChannel::AudioChannel(int stream_id,AVCodecContext *codecContext)
        : BaseChannel(stream_id,codecContext) {

}

void AudioChannel::decodeRender() {

}
