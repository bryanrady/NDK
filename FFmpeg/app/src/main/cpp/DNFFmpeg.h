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
#include "libavformat/avformat.h"
};

class DNFFmpeg{
public:
    DNFFmpeg(const char *data_source,JavaCallHelper *callHelper);
    ~DNFFmpeg();

    void prepare();
    void _prepare();

private:
    char *dataSource;
    pthread_t pid;
    AVFormatContext *formatContext;   //代表媒体文件的结构体，从这个结构体里面得到文件的各种信息(比如说宽高)
    JavaCallHelper *callHelper;
    AudioChannel *audioChannel;
    VideoChannel *videoChannel;
};

#endif //FFMPEG_DNFFMPEG_H
