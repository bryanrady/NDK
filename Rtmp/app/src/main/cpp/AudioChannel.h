//
// Created by Administrator on 2019/11/25.
//

#ifndef RTMP_AUDIOCHANNEL_H
#define RTMP_AUDIOCHANNEL_H

#include "librtmp/rtmp.h"
#include <faac.h>
#include <sys/types.h>

class AudioChannel{
    typedef void (*AudioCallback)(RTMPPacket *);
public:
    AudioChannel();
    ~AudioChannel();

    void setAudioCallback(AudioCallback callback);

    void setAudioEncInfo(int samplesInHZ, int channels);

    void encodeData(int8_t *data);

    int getInputSamples();

    RTMPPacket *getAudioTag();

private:
    AudioCallback audioCallback;
    //声道数
    int channels;
    //unsigned long 最大能输入编码器的样本数量
    u_long inputSamples;
    //最大可能的输出数据  编码后的最大字节数
    u_long maxOutputBytes;
    //编码器
    faacEncHandle audioCodec = 0;
    //unsigned char 输出缓冲区 编码后的数据 用这个缓冲区来保存
    u_char *outBuffer = 0;
};

#endif //RTMP_AUDIOCHANNEL_H
