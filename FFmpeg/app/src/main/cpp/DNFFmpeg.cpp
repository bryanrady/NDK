//
// Created by Administrator on 2019/11/9.
//

#include "DNFFmpeg.h"
#include <cstring>
#include "macro.h"

extern "C"{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

DNFFmpeg::DNFFmpeg(const char *data_source,JavaCallHelper *callHelper) {
    //这么写可能有问题，传进来的data_source可能被其他地方释放掉，而导致DNFFmpeg的成员dataSource指向一个
    //被释放的内存变成悬空指针
    //this->dataSource = const_cast<char *>(data_source);
     //这样做是为了防止data_source的内存被释放
     this->dataSource = new char[strlen(data_source)];
     strcpy(this->dataSource,data_source);
     this->callHelper = callHelper;
}

DNFFmpeg::~DNFFmpeg() {
    DELETE(dataSource);
    DELETE(callHelper);
}

void* task_prepare(void *args){
    DNFFmpeg *dnffmpeg = static_cast<DNFFmpeg *>(args);
    dnffmpeg->_prepare();

    //记住这里一定要return
    return 0;
}

void DNFFmpeg::prepare() {
    //创建一个线程，在这个线程里面进行dataSource的解析
    pthread_create(&pid,0,task_prepare,this);
}

/**
 * 解析dataSource 对媒体文件进行解封装流程 为音视频解码操作做准备
 *       1. av_register_all() 在FFmpeg4.0以上不用调用这个函数了
 *       2. avformat_open_input()   打开媒体文件播放地址
 *       3. avformat_find_stream_info() 查找媒体文件中的音视频流
 *              前面4个是处理音视频的形同操作，第5个才开始做差异化处理，也就是各自的解码过程
 *              (1). avcodec_find_decoder()
 *              (2). avcodec_alloc_context3()
 *              (3). avcodec_parameters_to_context()
 *              (4). avcodec_open2()
 *              (5). 针对音频和视频做差异性处理 就是准备解码了
 */
void DNFFmpeg::_prepare() {
    //1.先调用这个函数，FFmpeg才能够使用网络
    avformat_network_init();

    //开始解码
    //2.打开媒体文件播放地址
    formatContext = 0;
    //只有返回0才会成功，否则都是失败 失败原因：可能文件路径不对、没有网络等
    int ret = avformat_open_input(&formatContext,dataSource,0,0);
    if(ret != 0){
        LOGE("打开媒体失败：%s",av_err2str(ret));
        //打开媒体失败，我们就要通知java层失败
        callHelper->onError(THREAD_CHILD,FFMPEG_CAN_NOT_OPEN_URL);
        return;
    }

    //3.查找媒体文件中的音视频流 这一步操作就相当于给AVFormatContext赋值了
    ret = avformat_find_stream_info(formatContext,0);
    if(ret < 0){
        LOGE("查找媒体文件中的音视频流失败：%s",av_err2str(ret));
        callHelper->onError(THREAD_CHILD,FFMPEG_CAN_NOT_FIND_STREAMS);
        return;
    }
    //nb_streams 有几个流，就是有几个视频或者音频或者其他东西，可能不光只包含音视频流
    for(size_t i=0;i<formatContext->nb_streams;i++){
        //获取视频流 这段流可能是视频也可能是音频
        AVStream *avStream = formatContext->streams[i];
        //AVCodecParameters包含了解码这段流的各种信息
        AVCodecParameters *codecParameters = avStream->codecpar;
        //获取媒体类型 根据这个参数来判断是音频还是视频
        AVMediaType codec_type = codecParameters->codec_type;
        //获取流的编码方式
        AVCodecID avCodecId = codecParameters->codec_id;
        //音视频的相同处理
        //(1).通过当前流的编码方式来查找解码器
        AVCodec *avCodec = avcodec_find_decoder(avCodecId);
        if (avCodec == NULL){
            LOGE("查找音视频编码器失败");
            callHelper->onError(THREAD_CHILD,FFMPEG_FIND_DECODER_FAIL);
            return;
        }
        //(2).获得解码器上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(avCodec);
        if (codecContext == NULL){
            LOGE("获得解码器上下文失败");
            callHelper->onError(THREAD_CHILD,FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            return;
        }
        //(3)给解码器上下文设置一些参数,将codecParameters的一些参数复制给codecContext
        ret = avcodec_parameters_to_context(codecContext,codecParameters);
        if (ret < 0){
            LOGE("设置一些参数失败");
            callHelper->onError(THREAD_CHILD,FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            return;
        }
        //(4)打开解码器
        ret = avcodec_open2(codecContext,avCodec,0);
        if(ret != 0){
            LOGE("打开解码器失败");
            callHelper->onError(THREAD_CHILD,FFMPEG_OPEN_DECODER_FAIL);
            return;
        }

        //对音频和视频进行不同的处理，但是有些处理是相同的，我们写在上面
        if(codec_type == AVMEDIA_TYPE_AUDIO){   //音频
            audioChannel= new AudioChannel;
        }else if(codec_type == AVMEDIA_TYPE_VIDEO){ //视频
            videoChannel = new VideoChannel;
        }
    }

    //经过for循环查找之后，如果既没有音也没有视频(这种情况很少见，但是也要进行处理)
    if (audioChannel == NULL && videoChannel == NULL){
        callHelper->onError(THREAD_CHILD,FFMPEG_NOMEDIA);
        return;
    }

    //如果还能执行到这里，说明准备工作已经做好了，可以通知java随时可以播放
    callHelper->onPrepared(THREAD_CHILD);
}