//
// Created by wangqb on 2019/11/19.
//

#ifndef RTMP_VIDEOCHANNEL_H
#define RTMP_VIDEOCHANNEL_H

#include <x264.h>
#include <pthread.h>

class VideoChannel{
public:
    VideoChannel();
    ~VideoChannel();

    //创建x264编码器
    void setVideoEncInfo(int width, int height, int fps, int bitrate);
    //对视频数据进行编码
    void encodeData(int8_t *data);

private:
    pthread_mutex_t mutex;
    int width;
    int height;
    int fps;
    int bitrate;
    //x264编码器
    x264_t *videoCodec = 0;
};

#endif //RTMP_VIDEOCHANNEL_H
