//
// Created by Administrator on 2019/11/9.
//

#include "VideoChannel.h"
#include "macro.h"

extern "C"{
#include "libavutil/imgutils.h"
}

//在调用构造方法的时候 将streamId传给父类的stream_id,将avCodecContext传给父类的codecContext
VideoChannel::VideoChannel(int stream_id,AVCodecContext *codecContext)
        : BaseChannel(stream_id, codecContext) {
}

VideoChannel::~VideoChannel() {
}

void VideoChannel::setRenderFrameCallback(RenderFrameCallback callback) {
    this->renderFrameCallback = callback;
}

void* task_decode(void *args){
    VideoChannel *channel = static_cast<VideoChannel *>(args);
    channel->decode();
    return 0;
}

void* task_render(void *args){
    VideoChannel *channel = static_cast<VideoChannel *>(args);
    channel->render();
    return 0;
}

void VideoChannel::decodeRender() {
    //设置为正在播放
    isPlaying = 1;
    //将队列设置为工作状态
    packets.setWork(1);
    frames.setWork(1);
    //开启一个线程来进行解码
    pthread_create(&pid_decode,0,task_decode,this);
    //开启一个线程来进行播放
    pthread_create(&pid_render,0,task_render,this);
}

void VideoChannel::decode() {
    AVPacket *avPacket = 0;
    while (isPlaying){
        int ret = packets.pop(avPacket);
        if (!isPlaying){    //我们再取包的时候播放器可能停止了，直接break（因为取包的过程是个阻塞过程）
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
void VideoChannel::render(){
    swsContext = sws_getContext(
            codecContext->width,codecContext->height,codecContext->pix_fmt,
            codecContext->width,codecContext->height,AV_PIX_FMT_RGBA,
            SWS_BILINEAR,0,0,0
            );
    AVFrame *avFrame = 0;
    uint8_t * dst_data[4];  //指针数组
    int dst_linesize[4];
    //申请内存，要记得释放
    av_image_alloc(
            dst_data,dst_linesize,
            codecContext->width,codecContext->height,AV_PIX_FMT_RGBA,1
            );

    while (isPlaying){
        int ret = frames.pop(avFrame);
        if (!isPlaying){
            break;
        }
        if (ret == 0){
            continue;
        }
        //avFrame->linesize 每一行存放的图像字节长度
        sws_scale(
                swsContext,avFrame->data,
                avFrame->linesize,0,codecContext->height,
                dst_data,dst_linesize
                );
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