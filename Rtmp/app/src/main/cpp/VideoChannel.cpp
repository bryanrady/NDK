//
// Created by wangqb on 2019/11/19.
//

#include <x264.h>
#include "VideoChannel.h"

VideoChannel::VideoChannel() {
    pthread_mutex_init(&mutex,0);

}

VideoChannel::~VideoChannel() {
    pthread_mutex_destroy(&mutex);
}

void VideoChannel::setVideoEncInfo(int width, int height, int fps, int bitrate) {
    //这里需要用到互斥锁来进行同步
    //我们在推流的时候是在一个线程中使用编码器进行编码,然后另外一个线程可能会调用setVideoEncInfo来改变编码器，所以要保证线程安全
    pthread_mutex_lock(&mutex);
    this->width = width;
    this->height = height;
    this->fps = fps;
    this->bitrate = bitrate;

    //java层点击切换摄像头，就会调用到这个方法，所以如果编码器已经打开过了，就要把老的编码器进行释放
    if(videoCodec){
        x264_encoder_close(videoCodec);
        videoCodec = 0;
    }

    //配置x264编码器的属性    x264_param_t
    x264_param_t param;
    //配置x264编码器的一些属性
    //直播配置成最快的并且无延迟的编码 ultrafast 最快  zerolatency 无延迟编码
    x264_param_default_preset(&param,"ultrafast","zerolatency");
    //编码规格 3.2  可以在维基百科上面查找h.264,里面有一些说明
    param.i_level_idc = 32;
    //输入数据  使用I420的格式
    param.i_csp = X264_CSP_I420;
    //输入数据的宽高
    param.i_width = width;
    param.i_height = height;
    //设置编码无B帧   B帧是双向预测帧    P帧是前向预测帧    I帧是关键帧
    //虽然B帧编码出来的数据量小，但是会导致B帧的解码码花费的时间比较长，要去参考I帧和P帧
    param.i_bframe = 0;
    //码率控制  X264_RC_CQP（恒定质量）   X264_RC_CRF（恒定码率）    X264_RC_CRF（平均码率）
    param.rc.i_rc_method = X264_RC_ABR;
    //码率 （比特率 单位 kbps）
    param.rc.i_bitrate = bitrate/1000;
    //瞬时最大码率
    param.rc.i_vbv_max_bitrate = bitrate/1000 * 1.2;
    //码率控制区 设置了i_vbv_max_bitrate就必须设置i_vbv_buffer_size，单位kbps
    param.rc.i_vbv_buffer_size = bitrate/1000;
    //帧率 分子/分母
    param.i_fps_num = fps;
    param.i_fps_den = 1;
    //时间戳 分子/分母
    param.i_timebase_num = param.i_fps_num;
    param.i_timebase_den = param.i_fps_den;
    //用fps而不是用时间戳来计算帧间的距离
    param.b_vfr_input = 0;
    //两个关键帧的帧距离 这里就相当于2s输出一个关键帧
    //注意：如果是*100,在播放直播的时候，当前正在播放第40s的画面，但是来一个新的观众打开播放器看直播的时候，他就要等到60s以后
    //才能接收到sps与pps，然后才能看到画面,所以直播的时候不能设置帧间隔太久
    param.i_keyint_max = fps * 2;
    //是否复制sps和pps放在每个关键帧的前面,这个参数的设置是为了让每个关键帧I帧前面都附带有sps和pps 输出的数据就是 sps+pps+I帧，
    //这里如果给0的话，在每次要编码出I帧的时候就不会把sps与pps拷贝到关键帧前面。
    //必须把sps与pps就是交给解码器，解码器才能对图像进行解码，然后才能获得编码之前的图像(原始的rgb图像)，sps与pps就像密钥一样，只有知道了密钥才能知道如何进行解码
    param.b_repeat_headers = 1;
    //多线程
    param.i_threads = 1;
    //质量 baseline 这种规格是没有B帧的
    x264_param_apply_profile(&param,"baseline");

    //到这里参数就配置好了

    //打开x264编码器,后续就通过这个编码器进行编码
    videoCodec = x264_encoder_open(&param);

    pthread_mutex_unlock(&mutex);
}

void VideoChannel::encodeData(int8_t *data) {

}
