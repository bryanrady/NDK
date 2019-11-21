//
// Created by wangqb on 2019/11/19.
//

#ifndef RTMP_VIDEOCHANNEL_H
#define RTMP_VIDEOCHANNEL_H

#include <pthread.h>
#include "librtmp/rtmp.h"
//这里<x264.h>必须放在后面不知道为什么，要不然会报错
#include <x264.h>

class VideoChannel{
        typedef void (*VideoCallback)(RTMPPacket *);
public:
    VideoChannel();
    ~VideoChannel();

    //创建x264编码器
    void setVideoEncInfo(int width, int height, int fps, int bitrate);
    //对视频数据进行编码
    void encodeData(int8_t *data);

    void setVideoCallback(VideoCallback callback);

private:
    pthread_mutex_t mutex;
    int width;
    int height;
    int fps;
    int bitrate;
    //x264编码器
    x264_t *videoCodec = 0;
    //图片
    x264_picture_t *pic_in = 0;
    int ySize;
    int uvSize;
    VideoCallback videoCallback;

    void sendSpsPps(uint8_t *sps, uint8_t *pps, int sps_len, int pps_len);

    void sendFrame(int type, uint8_t *p_payload, int i_payload);
};

#endif //RTMP_VIDEOCHANNEL_H
