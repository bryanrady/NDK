//
// Created by Administrator on 2019/11/9.
//

#ifndef FFMPEG_DNFFMPEG_H
#define FFMPEG_DNFFMPEG_H

#include <pthread.h>
#include "JavaCallHelper.h"
#include "AudioChannel.h"
#include "VideoChannel.h"

extern "C"{
#include <libavformat/avformat.h>
};

class DNFFmpeg{
public:
    DNFFmpeg(const char *data_source,JavaCallHelper *callHelper);
    ~DNFFmpeg();

    void prepare();
    void _prepare();
    void start();
    void _start();
    void stop();
    void _stop();
    //设置播放渲染回调
    void setRenderFrameCallback(RenderFrameCallback callback);

    int getDuration();

    void seek(int progress);

    void pause();

private:
    char *dataSource;
    pthread_t pid_prepare;
    pthread_t pid_start;
    pthread_t pid_stop;
    AVFormatContext *formatContext = 0;   //代表媒体文件的结构体，从这个结构体里面得到文件的各种信息
    JavaCallHelper *callHelper; //这个在构造函数中已经赋值了，可以不用进行初始化
    AudioChannel *audioChannel = 0;
    VideoChannel *videoChannel = 0;
    bool isPlaying;
    RenderFrameCallback renderFrameCallback;
    int duration;
    bool isSeek;
    pthread_mutex_t seekMutex;
};

#endif //FFMPEG_DNFFMPEG_H
