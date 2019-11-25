//
// Created by Administrator on 2019/11/25.
//

#include "AudioChannel.h"
#include "macro.h"
#include <cstring>

AudioChannel::AudioChannel() {

}

AudioChannel::~AudioChannel() {
    if(audioCodec){
        faacEncClose(audioCodec);
        audioCodec = 0;
    }
    DELETE(outBuffer);
}

void AudioChannel::setAudioCallback(AudioCallback callback) {
    this->audioCallback = callback;
}

void AudioChannel::setAudioEncInfo(int samplesInHZ, int channels) {
    this->channels = channels;

    //第三个参数：一次最大能输入编码器的样本数量 也编码的数据的个数 (一个样本是16位 2字节)
    //第四个参数：最大可能的输出数据  编码后的最大字节数
    //打开faac编码器,后续就通过这个编码器进行编码成aac格式的数据
    audioCodec = faacEncOpen(samplesInHZ, channels, &inputSamples, &maxOutputBytes);
    //设置编码器参数
    faacEncConfigurationPtr configurationPtr = faacEncGetCurrentConfiguration(audioCodec);
    //指定为 mpeg4 标准
    configurationPtr->mpegVersion = MPEG4;
    //lc 标准
    configurationPtr->aacObjectType = LOW;
    //输入格式 16位
    configurationPtr->inputFormat = FAAC_INPUT_16BIT;
    // 编码出原始数据 既不是adts也不是adif
    configurationPtr->outputFormat = 0;
    faacEncSetConfiguration(audioCodec,configurationPtr);

    //输出缓冲区 编码后的数据 用这个缓冲区来保存
    outBuffer = new u_char[maxOutputBytes];
}

void AudioChannel::encodeData(int8_t *data) {
    //返回编码后数据字节的长度
    int byteLen = faacEncEncode(audioCodec, reinterpret_cast<int32_t *>(data), inputSamples,
            outBuffer, maxOutputBytes);
    if(byteLen > 0){
        int bodySize = 2 + byteLen;
        RTMPPacket *packet = new RTMPPacket;
        RTMPPacket_Alloc(packet,bodySize);

        //声道
        if (channels == 1) {
            packet->m_body[0] = 0xAE;   //单声道
        }else{
            packet->m_body[0] = 0xAF;   //双声道
        }
        //编码出的声音 都是 0x01
        packet->m_body[1] = 0x01;
        //音频数据
        memcpy(&packet->m_body[2], outBuffer, byteLen);

        packet->m_hasAbsTimestamp = 0;
        packet->m_nBodySize = bodySize;
        packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
        packet->m_nChannel = 0x11;
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;

        audioCallback(packet);
    }
}

int AudioChannel::getInputSamples() {
    return inputSamples;
}

RTMPPacket *AudioChannel::getAudioTag() {
//    unsigned char **ppBuffer
//    unsigned long *pSizeOfDecoderSpecificInfo
    u_char *ppBuffer = 0;
    u_long pSizeOfDecoderSpecificInfo;
    faacEncGetDecoderSpecificInfo(audioCodec, &ppBuffer, &pSizeOfDecoderSpecificInfo);

    int bodySize = 2 + pSizeOfDecoderSpecificInfo;
    RTMPPacket *packet = new RTMPPacket;
    RTMPPacket_Alloc(packet, bodySize);

    //声道
    if (channels == 1) {
        packet->m_body[0] = 0xAE;   //单声道
    }else{
        packet->m_body[0] = 0xAF;   //双声道
    }
    //编码出的声音 都是 0x01
    packet->m_body[1] = 0x01;
    //音频数据
    memcpy(&packet->m_body[2], ppBuffer, pSizeOfDecoderSpecificInfo);

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = bodySize;
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nChannel = 0x11;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    return packet;
}

