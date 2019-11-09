//
// Created by Administrator on 2019/11/9.
//

#include "VideoChannel.h"
#include "macro.h"

//在调用构造方法的时候 将stream_id传给父类的streamId
VideoChannel::VideoChannel(int streamId) : BaseChannel(stream_id) {

}

void* task_decode(void *args){
    VideoChannel *channel = static_cast<VideoChannel *>(args);
    channel->decode();
    DELETE(channel);
    return 0;
}

void* task_render(void *args){
    VideoChannel *channel = static_cast<VideoChannel *>(args);
    channel->render();
    DELETE(channel);
    return 0;
}

void VideoChannel::decodeRender() {
    pthread_create(&pid_decode,0,task_decode,this);
    pthread_create(&pid_render,0,task_render,this);
}

void VideoChannel::decode() {

}

void VideoChannel::render(){

}