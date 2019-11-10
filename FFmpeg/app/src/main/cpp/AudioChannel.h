//
// Created by Administrator on 2019/11/9.
//

#ifndef FFMPEG_AUDIOCHANNEL_H
#define FFMPEG_AUDIOCHANNEL_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "BaseChannel.h"

extern "C"{
#include "libswresample/swresample.h"
};

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int stream_id,AVCodecContext *codecContext);
    ~AudioChannel();

    //输出缓冲区
    uint8_t *output_data = 0;

    //音频解码与播放入口
    void play();
    //音频解码解码
    void audio_decode();
    //音频播放
    void audio_play();
    //获得PCM
    int getPcm();

private:
    pthread_t pid_audio_decode;
    pthread_t pid_audio_play;

    //引擎 SLObjectItf是一个指针的指针，要初始化
    SLObjectItf engineObject = 0;
    //引擎接口
    SLEngineItf engineInterface = 0;
    //混音器
    SLObjectItf outputMixObject = 0;
    //混音器效果接口
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = 0;
    //播放器
    SLObjectItf bqPlayerObject = 0;
    //播放器接口
    SLPlayItf bqPlayerInterface = 0;
    //播放器队列接口
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueueInterface = 0;

    //重采样
    SwrContext *swrContext = 0;

    int out_channels;       //声道数
    int out_16_samplesize;  //16位
    int out_sample_rate;    //采样率
};

#endif //FFMPEG_AUDIOCHANNEL_H
