//
// Created by wangqb on 2019/11/9.
//

#ifndef FFMPEG_BASECHANNEL_H
#define FFMPEG_BASECHANNEL_H

#include "safe_queue.h"
#include "JavaCallHelper.h"

extern "C"{
#include <libavcodec/avcodec.h>
};

//音频和视频的父类  用来处理音频和视频的相同操作
class BaseChannel{
public:
    BaseChannel(int stream_id,AVCodecContext *codecContext,AVRational time_base,JavaCallHelper *callHelper);
    //父类的析构函数一定要设置成为虚函数，否则不会执行子类的析构函数，可能造成内存泄漏
    virtual ~BaseChannel();

     //释放AVPacket 为什么用指针的指针?  指针的指针能够修改传递进来的指针的指向
    static void releaseAvPacket(AVPacket** packet);
    static void releaseAvPacket2(AVPacket*& packet);
    //释放AVFrame
    static void releaseAVFrame(AVFrame **avFrame);

    //用于音视频解码和播放(渲染)操作 纯虚函数 交给子类实现，相当于java的抽象方法
    virtual void play() = 0;
    virtual void stop() = 0;

    void startWork();
    void stopWork();
    void clearQueue();

    int stream_id;
    AVCodecContext *codecContext = 0;
    SafeQueue<AVPacket *> packets;  //存取经过解封装后得到的包
    SafeQueue<AVFrame *> frames;    //存取经过解码后的图像
    bool isPlaying; //判断是不是播放状态
    AVRational time_base;   //帧的基本时间单位
    double frameClock;
    JavaCallHelper *callHelper = 0;
};

#endif //FFMPEG_BASECHANNEL_H
