//
// Created by Administrator on 2019/11/9.
//

#ifndef FFMPEG_VIDEOCHANNEL_H
#define FFMPEG_VIDEOCHANNEL_H

#include "BaseChannel.h"
#include "AudioChannel.h"

extern "C"{
#include <libswscale/swscale.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
};

typedef void (*RenderFrameCallback)(uint8_t *,int,int,int);
//用来完成视频的解码和播放
class VideoChannel : public BaseChannel{
public:
    VideoChannel(int stream_id,AVCodecContext *codecContext,AVRational time_base,int fps);
    ~VideoChannel();

    //视频解码播放入口
    void play();
    //视频解码
    void video_decode();
    //视频渲染播放
    void video_render();

    void stop();

    void setRenderFrameCallback(RenderFrameCallback callback);

    void setAudioChannel(AudioChannel *audioChannel);

private:
    pthread_t pid_video_decode;
    pthread_t pid_video_render;
    SwsContext *swsContext = 0;
    RenderFrameCallback renderFrameCallback;
    int fps;
    AudioChannel *audioChannel;
};

#endif //FFMPEG_VIDEOCHANNEL_H
