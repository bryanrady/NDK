//
// Created by wangqb on 2019/11/9.
//

#ifndef FFMPEG_BASECHANNEL_H
#define FFMPEG_BASECHANNEL_H

#include "safe_queue.h"

extern "C"{
#include "include/libavcodec/avcodec.h"
};

//音频和视频的父类  用来处理音频和视频的相同操作
class BaseChannel{
public:
    BaseChannel(int stream_id,AVCodecContext *codecContext);
    //父类的析构函数一定要设置成为虚函数，否则不会执行子类的析构函数，可能造成内存泄漏
    virtual ~BaseChannel();

    /**
     * 释放AVPacket
     * 为什么用指针的指针?  指针的指针能够修改传递进来的指针的指向
     * @param AVPacket
     */
    static void releaseAvPacket(AVPacket** packet);
    static void releaseAvPacket2(AVPacket*& packet);

    //用于音视频解码和播放(渲染)操作 纯虚函数 交给子类实现，相当于java的抽象方法
    virtual void decodeRender() = 0;

    int stream_id;
    AVCodecContext *codecContext;
    SafeQueue<AVPacket *> packets;
    bool isPlaying; //判断是不是播放状态
};

#endif //FFMPEG_BASECHANNEL_H
