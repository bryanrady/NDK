//
// Created by Administrator on 2019/11/9.
//

#include "DNFFmpeg.h"
#include <cstring>
#include "macro.h"

DNFFmpeg::DNFFmpeg(const char *data_source,JavaCallHelper *callHelper) {
    //这么写可能有问题，传进来的data_source可能被其他地方释放掉，而导致DNFFmpeg的成员dataSource指向一个
    //被释放的内存变成悬空指针
    //this->dataSource = const_cast<char *>(data_source);
     //这样做是为了防止data_source的内存被释放
     this->dataSource = new char[strlen(data_source)+1];
     strcpy(this->dataSource,data_source);
     this->callHelper = callHelper;

    isPlaying = 0;
    duration = 0;
    pthread_mutex_init(&seekMutex, 0);
}

//因为DNFFmpeg是在子线程中释放的，所以不能在析构函数中释放JavaCallHelper，因为JavaCallHelper的析构函数用到了主线程的env
DNFFmpeg::~DNFFmpeg() {
    DELETE(dataSource);
    pthread_mutex_destroy(&seekMutex);
}

void* task_stop(void *args){
    DNFFmpeg *dnfFmpeg = static_cast<DNFFmpeg *>(args);
    dnfFmpeg->_stop();
    //这里是在子线程释放的DnfFmpeg
    DELETE(dnfFmpeg);
    return 0;
}

void DNFFmpeg::stop() {
    isPlaying = 0;
    //这里释放callHelper必须在主线程中释放，因为JavaCallHelper的析构方法用到了主线程的env，
    DELETE(callHelper);
    pthread_create(&pid_stop,0,task_stop,this);
}

void DNFFmpeg::_stop(){
    //等待prepare线程结束后再去执行下面的操作
    pthread_join(pid_prepare,0);
    pthread_join(pid_start,0);
    //等待这两个线程结束后我们才能去释放AVFormatContext，因为这两个线程都会使用到formatContext,如果直接释放了，可能会有问题
    if(formatContext){
        //先关闭流的读取然后再释放
        avformat_close_input(&formatContext);
        avformat_free_context(formatContext);
        formatContext = 0;
    }

    if(audioChannel){
        audioChannel->stop();
    }
    if(videoChannel){
        videoChannel->stop();
    }
    DELETE(audioChannel);
    DELETE(videoChannel);
}

void* task_prepare(void *args){
    DNFFmpeg *dnffmpeg = static_cast<DNFFmpeg *>(args);
    dnffmpeg->_prepare();
    //记住这里一定要return
    return 0;
}

void DNFFmpeg::prepare() {
    //创建一个线程，在这个线程里面进行dataSource的解析
    pthread_create(&pid_prepare,0,task_prepare,this);
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
    //2.打开媒体文件播放地址(文件地址或者直播流地址)
    //只有返回0才会成功，否则都是失败 失败原因：可能文件路径不对、没有网络等
    //第三个参数：指示我们打开媒体的格式(mp4/flv等格式)
    //第四个参数; 字典，像一个map集合
    formatContext = avformat_alloc_context();
    AVDictionary *options = 0;
    //设置超时时间 单位是微秒 5s
    av_dict_set(&options,"timeout","5000000",0);
    //avformat_open_input 这是个耗时操作
    int ret = avformat_open_input(&formatContext,dataSource,0,0);
    av_dict_free(&options);
    if(ret != 0){
        LOGE("打开媒体失败：%s",av_err2str(ret));
        //打开媒体失败，我们就要通知java层失败
        if (callHelper){
            callHelper->onError(THREAD_CHILD,FFMPEG_CAN_NOT_OPEN_URL);
        }
        return;
    }

    //3.查找媒体文件中的音视频流 这一步操作就相当于给AVFormatContext赋值了
    //avformat_find_stream_info 这是个耗时操作
    ret = avformat_find_stream_info(formatContext,0);
    if(ret < 0){
        LOGE("查找媒体文件中的音视频流失败：%s",av_err2str(ret));
        if (callHelper){
            callHelper->onError(THREAD_CHILD,FFMPEG_CAN_NOT_FIND_STREAMS);
        }
        return;
    }
    //视频时长（单位：微秒us，转换为秒需要除以1000000）
    duration = formatContext->duration / 1000000;
    //nb_streams 有几个流，就是有几个视频或者音频或者其他东西，可能不光只包含音视频流
    for(int i=0;i<formatContext->nb_streams;++i){
        //获取媒体视频流 这段流可能是视频也可能是音频
        AVStream *avStream = formatContext->streams[i];
        //AVCodecParameters包含了解码这段流的各种信息（流格式/宽高/码率/帧率等）
        AVCodecParameters *codecParameters = avStream->codecpar;
        //获取媒体类型 根据这个参数来判断是音频还是视频
        AVMediaType codec_type = codecParameters->codec_type;
        //获取流的编码方式
        AVCodecID avCodecId = codecParameters->codec_id;
        //音视频的相同处理
        //(1).通过当前流的编码方式来查找解码器
        AVCodec *avCodec = avcodec_find_decoder(avCodecId);
        if (!avCodec){
            LOGE("查找音视频编码器失败");
            if (callHelper){
                callHelper->onError(THREAD_CHILD,FFMPEG_FIND_DECODER_FAIL);
            }
            return;
        }
        //(2).获得解码器上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(avCodec);
        if (!codecContext){
            LOGE("获得解码器上下文失败");
            if (callHelper){
                callHelper->onError(THREAD_CHILD,FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            }
            return;
        }
        //(3)给解码器上下文设置一些参数,将codecParameters的一些参数复制给codecContext
        ret = avcodec_parameters_to_context(codecContext,codecParameters);
        if (ret < 0){
            LOGE("设置一些参数失败");
            if (callHelper){
                callHelper->onError(THREAD_CHILD,FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            }
            return;
        }
        //(4)打开解码器
        ret = avcodec_open2(codecContext,avCodec,0);
        if(ret != 0){
            LOGE("打开解码器失败");
            if (callHelper){
                callHelper->onError(THREAD_CHILD,FFMPEG_OPEN_DECODER_FAIL);
            }
            return;
        }
        //帧的基本时间单位（秒）
        AVRational time_base = avStream->time_base;
        //对音频和视频进行不同的处理，但是有些处理是相同的，我们写在上面
        if(codec_type == AVMEDIA_TYPE_AUDIO){   //音频
            audioChannel= new AudioChannel(i,codecContext,time_base,callHelper);  //这里将i传进去，后面会通过这个i来判断这个流是视频包还是音频包
        }else if(codec_type == AVMEDIA_TYPE_VIDEO){ //视频
            //平均帧率: 就是单位时间内需要显示多少个图像
            AVRational avg_frame_rate = avStream->avg_frame_rate;
            //分子除以分母，得到fps，也可以调用这个av_q2d()这个函数，内部实现也是分子/分母
            //int fps = avg_frame_rate.num / avg_frame_rate.num;
            int fps = av_q2d(avg_frame_rate);

            videoChannel = new VideoChannel(i,codecContext,time_base,callHelper,fps);
            videoChannel->setRenderFrameCallback(renderFrameCallback);
        }
    }

    //经过for循环查找之后，如果既没有音也没有视频(这种情况很少见，但是也要进行处理)
    if (!audioChannel && !videoChannel){
        if (callHelper){
            callHelper->onError(THREAD_CHILD,FFMPEG_NOMEDIA);
        }
        return;
    }

    //如果还能执行到这里，说明准备工作已经做好了，可以通知java随时可以播放
    if (callHelper){
        callHelper->onPrepared(THREAD_CHILD);
    }
}

void* task_start(void *args){
    DNFFmpeg *dnffmpeg = static_cast<DNFFmpeg *>(args);
    dnffmpeg->_start();
    return 0;
}

void DNFFmpeg::start() {
    isPlaying = 1;
    if(audioChannel){
        //调用音频解码播放
        audioChannel->play();
    }
    if(videoChannel){
        if(audioChannel){
            videoChannel->setAudioChannel(audioChannel);
        }
        //调用视频解码播放
        videoChannel->play();
    }
    //创建一个线程
    pthread_create(&pid_start,0,task_start,this);
}

/**
 * 专门用来读取媒体数据包(音视频数据包) 解码放到专门的类VideoChannel来进行操作
 */
void DNFFmpeg::_start() {
    while (isPlaying){
        //这里读取文件的时候可以做一下限制，如果读本地文件的时候一下子就读完了，可能会造成oom
        if(audioChannel && audioChannel->packets.size() > 100){
            //每次睡眠10毫秒
            av_usleep(1000 * 10);
            continue;
        }
        if(videoChannel && videoChannel->packets.size() > 100){
            av_usleep(1000 * 10);
            continue;
        }
        //锁住formatContext
        pthread_mutex_lock(&seekMutex);
        AVPacket *avPacket = av_packet_alloc();
        int ret = av_read_frame(formatContext,avPacket);
        pthread_mutex_unlock(&seekMutex);
        //0 if OK, < 0 on error or end of file
        if(ret == 0){
            //这里根据avPacket->stream_index(是一个流序号)和存进去的i来判断是音频包还是视频包
            if(audioChannel && avPacket->stream_index == audioChannel->stream_id){
                audioChannel->packets.push(avPacket);
            }else if(videoChannel && avPacket->stream_index == videoChannel->stream_id){
                videoChannel->packets.push(avPacket);
            }
        }else if(ret == AVERROR_EOF){   //读取完成了，但是可能还没有播放完
            if(!audioChannel->packets.empty() && !audioChannel->frames.empty()
               && !videoChannel->packets.empty() && !videoChannel->frames.empty()){
                break;
            }
            //为什么这里要让它继续循环，而不是sleep
            //如果是做直播，可以sleep,如果要支持点播也就是播放本地文件，我们拖动进度条后退的时候，也不会刷新了，可能就没有效果了
        }else{
            break;
        }
    }
    //循环退出后就执行音视频的stop
    isPlaying = 0;
    audioChannel->stop();
    videoChannel->stop();
}

void DNFFmpeg::setRenderFrameCallback(RenderFrameCallback callback){
    this->renderFrameCallback = callback;
}

int DNFFmpeg::getDuration() {
    return this->duration;
}

void DNFFmpeg::seek(int progress) {
//进去必须 在0- duration 范围之类
    if (progress< 0 || progress >= duration) {
        return;
    }
    if (!audioChannel && !videoChannel) {
        return;
    }
    if (!formatContext) {
        return;
    }
    pthread_mutex_lock(&seekMutex);
    //单位是 微妙
    int64_t seek = progress * 1000000;
    //seek到请求的时间 之前最近的关键帧
    // 只有从关键帧才能开始解码出完整图片
    av_seek_frame(formatContext, -1,seek, AVSEEK_FLAG_BACKWARD);
//    avformat_seek_file(formatContext, -1, INT64_MIN, seek, INT64_MAX, 0);
    // 音频、与视频队列中的数据 是不是就可以丢掉了？
    if (audioChannel) {
        //暂停队列
        audioChannel->stopWork();
        //可以清空缓存
//        avcodec_flush_buffers();
        audioChannel->clearQueue();
        //启动队列
        audioChannel->startWork();
    }
    if (videoChannel) {
        videoChannel->stopWork();
        videoChannel->clearQueue();
        videoChannel->startWork();
    }
    pthread_mutex_unlock(&seekMutex);
}