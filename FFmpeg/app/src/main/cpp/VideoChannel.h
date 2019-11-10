//
// Created by Administrator on 2019/11/9.
//

#ifndef FFMPEG_VIDEOCHANNEL_H
#define FFMPEG_VIDEOCHANNEL_H

#include "BaseChannel.h"

extern "C"{
#include "libswscale/swscale.h"
};

typedef void (*RenderFrameCallback)(uint8_t *,int,int,int);
//用来完成视频的解码和播放
class VideoChannel : public BaseChannel{
public:
    VideoChannel(int stream_id,AVCodecContext *codecContext);
    ~VideoChannel();

    //解码播放入口
    void decodeRender();
    //解码
    void decode();
    //渲染播放
    void render();
     //释放AVFrame
     static void releaseAVFrame(AVFrame **avFrame);

     void setRenderFrameCallback(RenderFrameCallback callback);

private:
    pthread_t pid_decode;
    pthread_t pid_render;
    SafeQueue<AVFrame *> frames;    //存取经过解码后的图像
    SwsContext *swsContext = 0;
    RenderFrameCallback renderFrameCallback;
};

#endif //FFMPEG_VIDEOCHANNEL_H
