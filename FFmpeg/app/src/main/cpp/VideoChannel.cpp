//
// Created by Administrator on 2019/11/9.
//

#include "VideoChannel.h"
#include "macro.h"

/**
 * 丢包 直到下一个关键帧 I帧出现  每次丢一组帧 I与I之间的帧
 * @param packets
 */
void dropAVPacket(queue<AVPacket *> &packets) {
    while (!packets.empty()) {
        AVPacket *packet = packets.front();
        //如果不属于 I 帧（关键帧） AVPacket里面的标签 flags
        if (packet->flags != AV_PKT_FLAG_KEY) {
            BaseChannel::releaseAvPacket(&packet);
            packets.pop();
        } else {
            break;
        }
    }
}

/**
 * 丢掉已经解码的图片  每次丢一张图片
 * @param frames
 */
void dropAVFrame(queue<AVFrame *> &frames){
    if (!frames.empty()) {
        AVFrame *frame = frames.front();
        frame->nb_samples;
        BaseChannel::releaseAVFrame(&frame);
        frames.pop();
    }
}

//在调用构造方法的时候 将streamId传给父类的stream_id,将avCodecContext传给父类的codecContext
VideoChannel::VideoChannel(int stream_id,AVCodecContext *codecContext,AVRational time_base,int fps)
        : BaseChannel(stream_id, codecContext,time_base) {
    this->fps = fps;

    packets.setSyncHandle(dropAVPacket);

    //设置回调 调用dropAVFrame丢包方法 这里我们来丢AVFrame包
    //这里进行同步，达到线程安全 不用担心多线程的问题
    frames.setSyncHandle(dropAVFrame);
}

VideoChannel::~VideoChannel() {
}

void VideoChannel::setRenderFrameCallback(RenderFrameCallback callback) {
    this->renderFrameCallback = callback;
}

void VideoChannel::setAudioChannel(AudioChannel *audioChannel) {
    this->audioChannel = audioChannel;
}

void* task_video_decode(void *args){
    VideoChannel *channel = static_cast<VideoChannel *>(args);
    channel->video_decode();
    return 0;
}

void* task_video_render(void *args){
    VideoChannel *channel = static_cast<VideoChannel *>(args);
    channel->video_render();
    return 0;
}

void VideoChannel::play() {
    //设置为正在播放
    isPlaying = 1;
    //将队列设置为工作状态
    packets.setWork(1);
    frames.setWork(1);
    //开启一个线程来进行解码
    pthread_create(&pid_video_decode,0,task_video_decode,this);
    //开启一个线程来进行播放
    pthread_create(&pid_video_render,0,task_video_render,this);
}

void VideoChannel::video_decode() {
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
                releaseAvPacket(&avPacket);
                break;
            }
        }
        //将得到的图像放进去图像队列中
        frames.push(avFrame);
    }
    //这里要记得释放，如果我们取包的时候暂停了或者停止了但是包已经取出来了，所以这时候avPacket的内存不在队列中了，
    // 所以即使调用队列的clear方法也不会释放avPacket的内存
    releaseAvPacket(&avPacket);
}

/**
 * 渲染播放 如何来播放AVFrame 将AVFrame转化成RGBA数据
 */
void VideoChannel::video_render(){
    swsContext = sws_getContext(
            codecContext->width,codecContext->height,codecContext->pix_fmt,
            codecContext->width,codecContext->height,AV_PIX_FMT_RGBA,
            SWS_BILINEAR,0,0,0);
    AVFrame *avFrame = 0;
    uint8_t * dst_data[4];  //指针数组
    int dst_linesize[4];
    //申请内存，要记得释放
    av_image_alloc(dst_data,dst_linesize,codecContext->width,codecContext->height,AV_PIX_FMT_RGBA,1);

    //得到每个图像画面的刷新间隔(播放间隔),所以fps越高，画面就会越流畅
    double frame_delays = 1.0/fps;

    while (isPlaying){
        int ret = frames.pop(avFrame);
        if (!isPlaying){
            releaseAVFrame(&avFrame);
            break;
        }
        if (ret == 0){
            continue;
        }
        //avFrame->linesize 每一行存放的图像字节长度
        sws_scale(swsContext,avFrame->data,
                avFrame->linesize,0,codecContext->height,
                dst_data,dst_linesize);
#if 1

        //这里音视频同步以 音频作为基准
        //获得当前帧avFrame画面的相对播放时间 (相对于开始播放的时间)，我们也可以通过pts来获得，但是一般在处理视频的情况不通过
        // pts来获得，通过best_effort_timestamp来获得，这两个有什么区别？其实大部分情况下这两个值是相等的，best_effort_timestamp
        // 比pts参考了更多的情况，比pts更加精准的来代表我们当前画面的相对播放时间
        double video_frame_clock = avFrame->best_effort_timestamp * av_q2d(time_base);
        //额外的间隔时间 repeat_pict 这个东西是当我们在进行解码操作的时候，这个图像需要延迟多久才显示，所以需要加上这个延迟
        double extra_delay = avFrame->repeat_pict / (2*fps);
        // 真实需要的间隔时间
        double delays = extra_delay + frame_delays;

        if (audioChannel != NULL){
            //如果第一个图像出来的时候，video_frame_clock可能为0，那就以正常的时间间隔来进行播放正常播放
            if (video_frame_clock == 0) {
                av_usleep(delays * 1000000);
            }else{
                //获得音频的相对播放时间 s
                double audio_frame_clock = audioChannel->frameClock;
                //音视频播放相差的间隔
                double diff = video_frame_clock - audio_frame_clock;
                if(diff > 0){
                    //视频快了 就要让视频的这一帧数据睡得更久一点，就是更新画面慢一点
                    LOGE("视频快了：%lf",diff);
                    av_usleep((delays + diff) * 1000000);
                }else if (diff < 0){
                    //视频慢了 就要快点赶上音频
                    LOGE("视频慢了：%lf",diff);
                    //有种情况：可能音频快了，要播放的视频包积压太多，赶不上音频了，如果这里进行音频睡眠的话，就会造成延迟越来越高，
                    //可能直播播到了第5，但是因为休眠的话可能还在播第2个，就会有很高的延迟，所以我们就要来丢包 丢视频包
                    //在丢包 AVPackeg包的时候，只能丢B帧跟P帧，不能丢I帧.如果丢掉了I帧，那么P帧和B帧的数据就解码不出来
                    //但是也不是不能丢掉I帧，比如说 一个队列里面存放的 帧数据 IBBPIBPI ，如果要丢掉第一个I的话，就要把后面的BBP也丢掉，
                    //直到找到下一次I帧，如果第一个I帧不丢的话，可以直接把后面的BBP都丢掉，这只是举个例子而已
                    // 因为P帧要解码的话，需要参考I帧的数据，B帧在解码的时候需要参考I帧和P帧的数据，I帧保存的是完整的图像数据
                    //所以I帧的数据两最大，P帧第2，B帧第3
                    //但是在解码的时间上 解码I帧花费时间最少，P帧其次，B帧最后
                    if(fabs(diff) >= 0.05){ //绝对值
                        LOGE("视频慢了开始丢包");
                        releaseAVFrame(&avFrame);
                        //丢包 调用队列的这个方法就会回调到队列的setSyncHandle()
                        frames.sync();
                        continue;
                    //    packets.sync();
                    }else{
                        //在允许的范围之内
                        av_usleep(delays * 1000000);
                    }
                }else{
                    //相等的话 以正常的时间间隔来进行播放正常播放
                   // LOGE("音视频播放时间间隔相等");
                   av_usleep(delays * 1000000);
                };
            }
        }else{
            //av_usleep(delays * 1000000);
        }
#endif
        releaseAVFrame(&avFrame);

        //通过上面的步骤，现在就转成了RGBA数据 存在了dst_data中,将转出来的数据回调出去进行播放
        //dst_data是一个指针数组，它将所有的数据都存储在第0个上面（123上面都没有数据的），所以我们直接将第0个回调出去即可，这就是为什么释放也只是释放第0个
        if(renderFrameCallback != NULL){
            renderFrameCallback(dst_data[0],dst_linesize[0],codecContext->width,codecContext->height);
        }
    }
    av_freep(&dst_data[0]);
    releaseAVFrame(&avFrame);
}