//
// Created by Administrator on 2019/11/9.
//

#ifndef FFMPEG_AUDIOCHANNEL_H
#define FFMPEG_AUDIOCHANNEL_H

#include "BaseChannel.h"

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int streamId);

    void decodeRender();
};

#endif //FFMPEG_AUDIOCHANNEL_H
