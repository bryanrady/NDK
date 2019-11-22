//
// Created by wangqb on 2019/11/19.
//

#include "VideoChannel.h"
#include "macro.h"

VideoChannel::VideoChannel() {
    pthread_mutex_init(&mutex,0);
}

VideoChannel::~VideoChannel() {
    pthread_mutex_destroy(&mutex);
    if(videoCodec){
        x264_encoder_close(videoCodec);
        DELETE(videoCodec);
    }
    if(pic_in){
        x264_picture_clean(pic_in);
        DELETE(pic_in);
    }
}

void VideoChannel::setVideoEncInfo(int width, int height, int fps, int bitrate) {
    //这里需要用到互斥锁来进行同步
    //我们在推流的时候是在一个线程中使用编码器进行编码,然后另外一个线程可能会调用setVideoEncInfo来改变编码器，所以要保证线程安全
    pthread_mutex_lock(&mutex);
    this->width = width;
    this->height = height;
    this->fps = fps;
    this->bitrate = bitrate;
    //Y U V 分量所占的数据大小
    // YUV420的内存计算
    //width * hight =Y（总和）
    //U = Y / 4   V = Y / 4
    ySize = width * height;
    uvSize = ySize / 4;

    //java层点击切换摄像头，就会调用到这个方法，所以如果编码器已经打开过了，就要把老的编码器进行释放
    if(videoCodec){
        x264_encoder_close(videoCodec);
        DELETE(videoCodec);
    }
    if(pic_in){
        x264_picture_clean(pic_in);
        DELETE(pic_in);
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

    pic_in = new x264_picture_t;
    x264_picture_alloc(pic_in, X264_CSP_I420, width, height);

    pthread_mutex_unlock(&mutex);
}

/**
 * 这里传过来的data是NV21格式的，编码器接收的是I420的，我们要转成I420格式的
 * 这两种格式Y数据是一样的，所以把Y数据复制过来就行了
 * @param data
 */
void VideoChannel::encodeData(int8_t *data) {
    pthread_mutex_lock(&mutex);
    //将data放入pic_in的各个平面中
    //1.把完整的y数据复制到第1个平面
    memcpy(pic_in->img.plane[0], data, ySize);
    //2.把所有u、v数据分别复制到第2、3个平面
    for (int i = 0; i < uvSize; ++i) {
        //通过指针运算来取数据
        //u数据 间隔1个字节取一个数据   plane是一个指针数组，通过指针运算来进行赋值
        *(pic_in->img.plane[1] + i) = *(data + ySize + i * 2 + 1);
        //v数据
        *(pic_in->img.plane[2] + i) = *(data + ySize + i * 2);
    }
    //编码出的数据
    x264_nal_t *pp_nal = 0;
    //编码出了几个 nalu （暂时理解为帧）
    int pi_nal;
    x264_picture_t pic_out;
    //编码
    int ret = x264_encoder_encode(videoCodec, &pp_nal, &pi_nal, pic_in, &pic_out);
    if (ret < 0) {
        pthread_mutex_unlock(&mutex);
        return;
    }
    //这里将sps与pps保存到数组中，因为这两个的数据不可能很大，所以用一个差不多的空间保存下来
    int sps_len;
    int pps_len;
    uint8_t sps[100];
    uint8_t pps[100];
    for (int i = 0; i < pi_nal; ++i) {
        //编码出的数据的数据类型
        if(pp_nal[i].i_type == NAL_SPS){
            //这里为什么要-4,然后+4？
            //因为x264编码器在编码h264格式的数据的时候，数据之间是有间隔符号分开的, 而这个间隔符号就占了4个字节,
            //所以这里我们在取sps与pps数据的时候，就不需要把间隔符取出来
            sps_len = pp_nal[i].i_payload - 4;
            memcpy(sps, pp_nal[i].p_payload + 4, sps_len);
        }else if(pp_nal[i].i_type == NAL_PPS){
            pps_len = pp_nal[i].i_payload - 4;
            memcpy(pps, pp_nal[i].p_payload + 4, pps_len);
            //拿到pps 就表示 sps已经拿到了，因为pps是紧跟在sps后面的
            //将sps与pps推到服务器
            sendSpsPps(sps, pps, sps_len, pps_len);
        }else{
            sendFrame(pp_nal[i].i_type, pp_nal[i].p_payload, pp_nal[i].i_payload);
        }
    }

    pthread_mutex_unlock(&mutex);
}

//前面4位（0010）表示帧类型（Frame Type）（1: 关键帧	2: 普通帧）
//
//  1	keyframe (for AVC, a seekable frame) 关键帧
//  2	inter frame (for AVC, a non-seekable frame)
//  3	disposable inter frame (H.263 only)
//  4	generated keyframe (reserved for server use only)
//  5	video info/command frame
//
//后面4位（0010）表示编码ID（CodecID），这里是2，我们也可以看出我们的视频编码格式是使用的H.263
//
//  1	JPEG (currently unused)
//  2	Sorenson H.263
//  3	Screen video
//  4	On2 VP6
//  5	On2 VP6 with alpha channel
//  6	Screen video version 2
//  7	AVC

//整个数据区内容：  AVCPacketType == 0
//| 类型                     | 字节 | 说明                                          |
//| ------------------------ | ---- | --------------------------------------------- |
//| 视频信息                 |  1   | 包含帧类型和编码ID                             |
//| AVCPacketType            |  1   | AVC包类型                                      |
//| Composition Time         |  3   | 数据合成时间                                    |

//| 版本                     |  1    | 0x01                                          |
//| 编码规格                 |  3    | sps[1]+sps[2]+sps[3] (后面说明)               |
//| 几个字节表示 NALU 的长度 |  1    | 0xFF，包长为 （0xFF& 3） + 1，也就是4字节表示 |
//| SPS个数                  |  1    | 0xE1，个数为0xE1 & 0x1F 也就是1               |
//| SPS长度                  |  2    | 整个sps的长度                                 |
//| sps的内容                |  n    | 整个sps                                       |
//| pps个数                  |  1    | 0x01，不用计算就是1                           |
//| pps长度                  |  2    | 整个pps长度                                   |
//| pps内容                  |  n    | 整个pps内容

void VideoChannel::sendSpsPps(uint8_t *sps, uint8_t *pps, int sps_len, int pps_len) {
    //组装RTMP包
    RTMPPacket *packet = new RTMPPacket;
    //数据总长度
    int bodySize = 1 + 1 + 3 + 1 + 3 + 1 + 1 + 2 + sps_len + 1 + 2 + pps_len;
    //申请空间
    RTMPPacket_Alloc(packet,bodySize);

    int i = 0;
    //视频信息   1 关键帧  7 AVC(H264)编码
    packet->m_body[i++] = 0x17;
    //AVCPacketType     0
    packet->m_body[i++] = 0x00;
    //Composition Time
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;

    //版本
    packet->m_body[i++] = 0x01;
    //编码规格
    packet->m_body[i++] = sps[1];
    packet->m_body[i++] = sps[2];
    packet->m_body[i++] = sps[3];
    //几个字节表示 NALU 的长度
    packet->m_body[i++] = 0xFF;

    //sps个数
    packet->m_body[i++] = 0xE1;
    //sps长度
    packet->m_body[i++] = (sps_len >> 8) & 0xff;
    packet->m_body[i++] = sps_len & 0xff;
    //sps的内容    将整个sps复制过来
    memcpy(&packet->m_body[i], sps, sps_len);
    i += sps_len;

    //pps个数
    packet->m_body[i++] = 0x01;
    //pps长度
    packet->m_body[i++] = (pps_len >> 8) & 0xff;
    packet->m_body[i++] = (pps_len) & 0xff;
    //pps内容     将整个pps复制过来
    memcpy(&packet->m_body[i], pps, pps_len);

    //视频
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = bodySize;
    //随意分配一个管道（尽量避开rtmp.c中使用的）
    packet->m_nChannel = 10;
    //sps pps没有时间戳
    packet->m_nTimeStamp = 0;
    //不使用绝对时间
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;

    //通过这个函数指针回调出去
    videoCallback(packet);
}

void VideoChannel ::setVideoCallback(VideoCallback callback) {
    this->videoCallback = callback;
}

//整个数据区内容：AVCPacketType == 1
//| 类型                     | 字节 | 说明                                          |
//| ------------------------ | ---- | --------------------------------------------- |
//| 视频信息                 |  1   | 包含帧类型和编码ID                             |
//| AVCPacketType            |  1   | AVC包类型                                      |
//| Composition Time         |  3   | 数据合成时间                                    |

//| 版本                     |  1    | 0x01                                          |
//| 编码规格                 |  3    | sps[1]+sps[2]+sps[3] (后面说明)               |
//| 几个字节表示 NALU 的长度 |  1    | 0xFF，包长为 （0xFF& 3） + 1，也就是4字节表示 |
//| SPS个数                  |  1    | 0xE1，个数为0xE1 & 0x1F 也就是1               |
//| SPS长度                  |  2    | 整个sps的长度                                 |
//| sps的内容                |  n    | 整个sps                                       |
//| pps个数                  |  1    | 0x01，不用计算就是1                           |
//| pps长度                  |  2    | 整个pps长度                                   |
//| pps内容                  |  n    | 整个pps内容
void VideoChannel::sendFrame(int type, uint8_t *p_payload, int i_payload) {
    //去掉分隔符 00 00 00 01 / 00 00 01
    if (p_payload[2] == 0x00){
        i_payload -= 4;
        p_payload += 4;
    } else if(p_payload[2] == 0x01){
        i_payload -= 3;
        p_payload += 3;
    }
    //组装RTMP包
    RTMPPacket *packet = new RTMPPacket;
    int bodySize = 9 + i_payload;
    RTMPPacket_Alloc(packet, bodySize);
//    RTMPPacket_Reset(packet);

    int i = 0;
    //视频信息
    if (type == NAL_SLICE_IDR) {
        packet->m_body[i++] = 0x17;
        LOGE("关键帧");
    }else{
        packet->m_body[i++] = 0x27;
    }
    //AVCPacketType     1
    packet->m_body[i++] = 0x01;
    //时间戳
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    //图片数据长度 int 4个字节 相当于把int转成4个字节的byte数组
    packet->m_body[i++] = (i_payload >> 24) & 0xff;
    packet->m_body[i++] = (i_payload >> 16) & 0xff;
    packet->m_body[i++] = (i_payload >> 8) & 0xff;
    packet->m_body[i++] = (i_payload) & 0xff;
    //图片数据 将图片数据全部进行拷贝
    memcpy(&packet->m_body[i], p_payload, i_payload);

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = bodySize;
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nChannel = 0x10;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;

    //通过这个函数指针回调出去
    videoCallback(packet);
}
