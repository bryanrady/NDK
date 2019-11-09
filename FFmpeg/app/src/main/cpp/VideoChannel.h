//
// Created by Administrator on 2019/11/9.
//

#ifndef FFMPEG_VIDEOCHANNEL_H
#define FFMPEG_VIDEOCHANNEL_H

#include "BaseChannel.h"

//用来完成视频的解码和播放
class VideoChannel : public BaseChannel{
public:
    VideoChannel(int streamId);

    void decodeRender();
    //解码
    void decode();
    //播放
    void render();

private:
    pthread_t pid_decode;
    pthread_t pid_render;
};

#endif //FFMPEG_VIDEOCHANNEL_H
