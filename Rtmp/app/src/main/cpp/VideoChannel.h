//
// Created by wangqb on 2019/11/19.
//

#ifndef RTMP_VIDEOCHANNEL_H
#define RTMP_VIDEOCHANNEL_H

class VideoChannel{
public:
    VideoChannel();
    ~VideoChannel();

    void setVideoEncInfo(int width, int height, int fps, int bitrate);
};

#endif //RTMP_VIDEOCHANNEL_H
