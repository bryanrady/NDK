//
// Created by Administrator on 2019/11/9.
//

#include "AudioChannel.h"
#include "macro.h"

AudioChannel::AudioChannel(int stream_id,AVCodecContext *codecContext,AVRational time_base,JavaCallHelper *callHelper)
        : BaseChannel(stream_id,codecContext,time_base,callHelper) {
    //根据布局获取声道数
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    //16位数据 也就是2个字节
    out_16_samplesize = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    out_sample_rate = 44100;    //采样率
    //采样率44100  44100 * 2表示多少个16位数据 44100 * 2(16位) * 2(声道数)     44100 * 2(16位)代表1个16位数据
    output_data = static_cast<uint8_t *>(malloc(out_sample_rate * out_16_samplesize * out_channels));
    //记得free
    memset(output_data,0,out_sample_rate * out_16_samplesize * out_channels);
}

AudioChannel::~AudioChannel() {
    if (output_data){
        free(output_data);
        output_data = 0;
    }
}

//这里没有什么太耗时的操作，就不用开线程进行处理了
void AudioChannel::stop() {
    isPlaying = 0;
    callHelper = 0;
    //这里调用setWork(0) 就会通知不会继续等待了，所以这里setWork(0)后从队列中取数据就不会卡住了
    stopWork();
    pthread_join(pid_audio_decode,0);
    pthread_join(pid_audio_play,0);
    if(swrContext){
        swr_free(&swrContext);
        swrContext = 0;
    }

    //设置停止状态
    if (bqPlayerInterface) {
        (*bqPlayerInterface)->SetPlayState(bqPlayerInterface, SL_PLAYSTATE_STOPPED);
        bqPlayerInterface = 0;
    }

    if(bqPlayerObject){
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = 0;
        bqPlayerInterface = 0;
        bqPlayerBufferQueueInterface = 0;
    }
    if(outputMixObject){
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = 0;
    }
    if(engineObject){
        (*engineObject)->Destroy(engineObject);
        engineObject = 0;
        engineInterface = 0;
    }

}

void* task_audio_decode(void *args){
    AudioChannel *channel = static_cast<AudioChannel *>(args);
    channel->audio_decode();
    return 0;
}

void* task_audio_play(void *args){
    AudioChannel *channel = static_cast<AudioChannel *>(args);
    channel->audio_play();
    return 0;
}

void AudioChannel::play() {
    //第一个参数是swrContext *,可以传一个0
    //0+输出声道+输出采样位+输出采样率+  输入的3个参数
    swrContext = swr_alloc_set_opts(0,AV_CH_LAYOUT_STEREO,AV_SAMPLE_FMT_S16,out_sample_rate,
                                    codecContext->channel_layout, codecContext->sample_fmt,
                                    codecContext->sample_rate, 0, 0);
    //切记初始化swrContext 这里开始没有进行初始化，导致没有声音
    swr_init(swrContext);

    //设置为正在播放
    isPlaying = 1;
    //将队列设置为工作状态
    startWork();

    //开启一个线程来进行解码
    pthread_create(&pid_audio_decode,0,task_audio_decode,this);
    //开启一个线程来进行播放
    pthread_create(&pid_audio_play,0,task_audio_play,this);
}

//音频解码操作和视频解码操作是一样的
void AudioChannel::audio_decode() {
    AVPacket *avPacket = 0;
    while (isPlaying){
        int ret = packets.pop(avPacket);
        if (!isPlaying){    //我们再取包的时候播放器可能停止了，直接break（因为取包的过程是个阻塞过程）
            releaseAvPacket(&avPacket);
            break;
        }
        if(ret == 0){   //如果没有取成功就继续来取
            continue;
        }
        //取出来包之后，就把包丢给解码器
        ret = avcodec_send_packet(codecContext,avPacket);
        releaseAvPacket(&avPacket);
        if(ret != 0){ //失败
            if (ret == AVERROR(EAGAIN)){
                //这里的AVERROR(EAGAIN)意思是:丢给解码器里面的数据太多了，叫我们马上进行读取，以便于给解码器腾出空间继续解码新数据,
                //所以这里我们直接进行读取就行了，和成功的操作一样,所以这里可以直接不管
            }else{
                break;
            }
        }
        //AVFrame就代表了图像，这里和AVPacket的原理是一样的，记得手动释放
        AVFrame *avFrame = av_frame_alloc();
        //从解码器中读取解码后的数据包
        ret = avcodec_receive_frame(codecContext,avFrame);
        if (ret != 0){  //失败
            if (ret == AVERROR(EAGAIN)){
                //这里的AVERROR(EAGAIN)意思是:从解码器中读取的数据包太少，导致不够生成一段图像，需要更多的数据包才能生存图像
                continue;
            }else{
                break;
            }
        }
        while (frames.size() > 100 && isPlaying) {
            av_usleep(1000 * 10);
            continue;
        }
        //将得到的图像放进去图像队列中
        frames.push(avFrame);
    }
    //这里要记得释放，如果我们取包的时候暂停了或者停止了但是包已经取出来了，所以这时候avPacket的内存不在队列中了，
    // 所以即使调用队列的clear方法也不会释放avPacket的内存
    releaseAvPacket(&avPacket);
}

//从frames队列获取pcm数据，返回获取到的pcm数据大小
int AudioChannel::getPcm() {
    int dataSize = 0;
    AVFrame *avFrame = 0;
    int ret = frames.pop(avFrame);
    if(!isPlaying){
        if(ret != 0){   //这里如果不播了，并且数据获取成功了就来释放一下
            releaseAVFrame(&avFrame);
        }
        return 0;
    }
    //这里我们的播放器播放的数据格式是下面定义的pcm格式：我们这里从队列获取的pcm数据可能和播放器的数据格式不匹配
    //所以我们这里为了让数据格式匹配，重新对数据进行重采样
    //比如说：我们从队列中获取的数据格式是 48000HZ 8位 然后我们要把它转成播放器可播放的格式 44100 16位，这就是重采样过程
    //48000HZ 8位 =》 44100 16位 怎么来重采样呢？ FFmpeg提供了重采样的方法

    // 注意: 不是输入了多少数据，我们播放器就马上会处理这么多个数据
    // 假设我们输入了10个数据 ，swrContext转码器 这一次处理了8个数据，还剩下2个，所以我们要把上一次未处理的2个也添加进来
    // delays 就代表剩下的数据，那么如果不加delays(上次没处理完的数据) , 会造成积压, 然后会导致崩栈
    if(swrContext && avFrame){
        int64_t delays = swr_get_delay(swrContext,avFrame->sample_rate);
        // 将 nb_samples 个数据 由 sample_rate采样率转成 44100 后 返回output_max_samples个数据  相当于 m 个 nb_samples = output_max_samples 个 44100；
        // AV_ROUND_UP : 向上取整 1.1 = 2
        int64_t output_max_samples = av_rescale_rnd(avFrame->nb_samples+delays,out_sample_rate,avFrame->sample_rate,AV_ROUND_UP);
        //swrContext上下文+输出缓冲区+输出缓冲区能接受的最大数据量+输入数据+输入数据个数
        //返回真正转换的多少个数据  samples单位: 44100*2(声道数)
        int samples = swr_convert(swrContext, &output_data, output_max_samples, (const uint8_t **)(avFrame->data),avFrame->nb_samples);
        //获得多少个16位数据  ==  samples * out_samplesize * 声道数   16位数据就是2个字节数据
        //获得多少个字节大小  ==  samples * out_samplesize * 声道数 * 2
        //获取out_channels个声道输出的16位数据
        dataSize = samples * out_16_samplesize * out_channels;

        //pts  获得当前帧AVFrame 的一个相对播放时间 (相对开始播放的时间)
        //获得音频 相对播放这一段AVFrame数据的秒数
        //frameClock = avFrame->pts * av_q2d(time_base);
        frameClock = avFrame->best_effort_timestamp * av_q2d(time_base);
        LOGE("音频 frameClock: %d",frameClock);
        if (callHelper) {
            callHelper->onProgress(THREAD_CHILD, frameClock);
        }
    }
    releaseAVFrame(&avFrame);
    return dataSize;
}

//播放
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bqInterface, void *context) {
    AudioChannel *channel = static_cast<AudioChannel *>(context);
    //获得pcm 数据  上面返回的是多少个16位数据
    int dataSize = channel->getPcm();
    if(dataSize > 0 ){
        // 将pcm数据通过队列接口加入到队列中，就会自动播放了，接收16位数据
        (*bqInterface)->Enqueue(bqInterface,channel->output_data,dataSize);
    }
}

//使用OpenSL ES 来播放PCM音频原始数据
void AudioChannel::audio_play() {
    //1、创建引擎并获取引擎接口
    //(1)创建引擎
    SLresult result;
    result = slCreateEngine(&engineObject,0,NULL,0,NULL,NULL);
    if (result != SL_RESULT_SUCCESS){
        LOGE("创建引擎失败");
        return;
    }
    //(2)初始化引擎
    result = (*engineObject)->Realize(engineObject,SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS){
        LOGE("初始化引擎失败");
        return;
    }
    //(3)获取引擎接口SLEngineItf engineInterface
    result = (*engineObject)->GetInterface(engineObject,SL_IID_ENGINE,&engineInterface);
    if (result != SL_RESULT_SUCCESS){
        LOGE("获取引擎接口失败");
        return;
    }
    //2.设置混音器
    //(1)创建混音器
    result = (*engineInterface)->CreateOutputMix(engineInterface,&outputMixObject,0,0,0);
    if (result != SL_RESULT_SUCCESS){
        LOGE("创建混音器失败");
        return;
    }
    //(2)初始化混音器
    result = (*outputMixObject)->Realize(outputMixObject,SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS){
        LOGE("初始化混音器失败");
        return;
    }
    //(3)还可以设置混音效果,效果设置不设置都行
//    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,&outputMixEnvironmentalReverb);
//    if (SL_RESULT_SUCCESS == result) {
//        //SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT : 默认
//        //SL_I3DL2_ENVIRONMENT_PRESET_ROOM : 室内
//        //SL_I3DL2_ENVIRONMENT_PRESET_AUDITORIUM : 礼堂 等
//        const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
//        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(outputMixEnvironmentalReverb, &reverbSettings);
//        if (result != SL_RESULT_SUCCESS){
//            LOGE("设置混音效果失败");
//            return;
//        }
//    }else{
//        LOGE("获取混音器接口失败");
//        return;
//    }
    //3.创建播放器
    //(1)指定输入声音信息
    //创建buffer缓冲类型的队列 2个队列
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
    //pcm数据格式: pcm + 2(双声道) + 44100(采样率 1秒采集多少声音) + 16(采样位)
    //  + 16(数据的大小) + LEFT|RIGHT(双声道) + 小端数据
    //pcm数据格式:
    //SL_DATAFORMAT_PCM：数据格式为pcm格式
    //2：双声道
    //SL_SAMPLINGRATE_44_1：采样率为44100
    //SL_PCMSAMPLEFORMAT_FIXED_16：采样格式为16bit
    //SL_PCMSAMPLEFORMAT_FIXED_16：数据大小为16bit
    //SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT：左右声道（双声道）
    //SL_BYTEORDER_LITTLEENDIAN：小端模式
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM,2, SL_SAMPLINGRATE_44_1, SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                            SL_BYTEORDER_LITTLEENDIAN};
    //数据源 将上述配置信息放到这个数据源中
    SLDataSource slDataSource = {&android_queue, &pcm};

    //(2)配置音轨(输出)，就是通过outputMixObject这个混音器来输出声音的
    //把混音器装到SLDataLocator_OutputMix这个结构体里面
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    //音轨输出 再把SLDataLocator_OutputMix装到SLDataSink结构体里面
    SLDataSink audioSnk = {&outputMix, NULL};
    //需要的接口,下面就设置成SL_BOOLEAN_TRUE  操作队列的接口 SL_IID_BUFFERQUEUE，这里不需要混音接口
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    //(3) 创建播放器
    result = (*engineInterface)->CreateAudioPlayer(engineInterface, &bqPlayerObject, &slDataSource, &audioSnk, 1, ids, req);
    if (result != SL_RESULT_SUCCESS){
        LOGE("创建播放器失败");
        return;
    }
    //(4)初始化播放器
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS){
        LOGE("初始化播放器失败");
        return;
    }
    //(5)获取播放器接口来设置播放状态
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerInterface);
    if (result != SL_RESULT_SUCCESS){
        LOGE("获取Player播放接口失败");
        return;
    }
    //4.设置播放回调
    //(1)获取播放器队列接口
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueueInterface);
    if (result != SL_RESULT_SUCCESS){
        LOGE("获取播放器队列接口失败");
        return;
    }
    //(2)注册播放回调
    result = (*bqPlayerBufferQueueInterface)->RegisterCallback(bqPlayerBufferQueueInterface, bqPlayerCallback, this);
    if (result != SL_RESULT_SUCCESS){
        LOGE("注册播放回调失败");
        return;
    }
    //5.设置播放状态
    result = (*bqPlayerInterface)->SetPlayState(bqPlayerInterface, SL_PLAYSTATE_PLAYING);
    if (result != SL_RESULT_SUCCESS){
        LOGE("设置播放状态失败");
        return;
    }
    //6.启动播放回调函数 手动的激活一下回调
    bqPlayerCallback(bqPlayerBufferQueueInterface, this);

}
