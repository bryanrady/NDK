package com.bryanrady.douyin.record;

import android.content.Context;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.opengl.EGLContext;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.view.Surface;

import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * 自己创建一个MediaRecorder来进行视频录制
 */
public class MediaRecorder {

    private final Context mContext;
    private final String mPath;
    private final int mWidth;
    private final int mHeight;
    private final EGLContext mEglContext;   //绘制线程的EGL上下文,从渲染器中拿
    private float mSpeed;
    private int mBitrate = 1500_000;
    private int mFrameRate = 20;
    private MediaCodec mMediaCodec;
    private Surface mInputSurface;
    private MediaMuxer mMediaMuxer;
    private Handler mHandler;
    private EGLBase mEglBase;
    private boolean mIsStart = false;
    private int mIndex;


    /**
     * 除了这些参数外，还可以让码率 帧率等参数
     * @param context
     * @param path      录制后的视频要保存的地址
     * @param width     视频宽度
     * @param height    视频高度
     * @param eglContext
     */
    public MediaRecorder(Context context, String path, int width, int height, EGLContext eglContext){
        this.mContext = context.getApplicationContext();
        this.mPath = path;
        this.mWidth = width;
        this.mHeight = height;
        this.mEglContext = eglContext;
    }

    public void setBitRate(int bitrate){
        mBitrate = bitrate;
    }

    public void setFrameRate(int frameRate) {
        this.mFrameRate = frameRate;
    }

    /**
     * 开始录制视频
     * @param speed
     * @throws IOException
     */
    public void start(float speed) throws IOException{
        mSpeed = speed;

        /**
         * 配置MediaCodec 编码器
         */
        //视频格式 类型（avc高级编码 h264） 编码出的宽、高
        MediaFormat videoFormat = new MediaFormat();
        //参数配置
        //使用H264编码（avc高级编码 h264）
        videoFormat.setString(MediaFormat.KEY_MIME, MediaFormat.MIMETYPE_VIDEO_AVC);
        //宽
        videoFormat.setInteger(MediaFormat.KEY_WIDTH, mWidth);
        //高
        videoFormat.setInteger(MediaFormat.KEY_HEIGHT, mHeight);
        //码率
        videoFormat.setInteger(MediaFormat.KEY_BIT_RATE, mBitrate);
        //帧率
        videoFormat.setInteger(MediaFormat.KEY_FRAME_RATE, mFrameRate);
        //关键帧间隔
        videoFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 20);
        //设置视频输入颜色格式，这里选择使用Surface作为输入，可以忽略颜色格式的问题，并且不需要直接操作输入缓冲区。
        videoFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);

        //创建编码器
        mMediaCodec = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_VIDEO_AVC);
        //将参数配置交给编码器
        mMediaCodec.configure(videoFormat,null,null, MediaCodec.CONFIGURE_FLAG_ENCODE);

        //创建Surface 然后交给虚拟屏幕 通过OpenGL 将预览的纹理 绘制到这一个虚拟屏幕中
        //这样MediaCodec 就会自动编码 inputSurface 中的图像
        mInputSurface = mMediaCodec.createInputSurface();

        //播放流程：mp4文件--》解封装(解复用)--》得到音频流和视频流--》对流进行解码--》绘制
        //new MediaExtractor(); 解封装器(解复用器)
        //现在我们就是反过来了， 进行编码后， 然后就要通过封装器进行封装
        //创建封装器(复用器)   一个 mp4 的封装器 将h.264 通过它写到文件中就可以了
        mMediaMuxer = new MediaMuxer(mPath, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);

        /**
         *
         * 这里需要进行线程通信，为什么需要？
         *      我们要启动一个子线程来进行录制视频，当我们在GLThread线程处理好一帧图像后，我们要把这一帧图像传递到MediaRecoder中，
         *      然后通过MediaRecoder绘制到虚拟屏幕中，我们在绘制到虚拟屏幕中的时候，不能在GLThread线程中，我们要自己
         *      创建一个线程来进行绘制到虚拟屏幕中操作，所以会有一步线程间通信操作。
         *
         * 线程间的通信 Handler  而我们一般在使用Handler进行线程通信的时候，用法都是子线程通过handler.post去通知主线程，
         *     但是这里的场景是 GLThread线程(子线程) 通知 子线程(就是EGL的绑定线程)，
         *      所以我们要来创建一个HandlerThread，通过HandlerThread来获取属于子线程的Looper
         *
         */
        HandlerThread handlerThread = new HandlerThread("VideoCodec");
        handlerThread.start();
        Looper threadLooper = handlerThread.getLooper();
        //然后通过Handler来进行通信 这个Handler是用来GLThread线程通知EGL绑定线程
        //EGL的绑定线程  对我们自己创建的EGL环境的opengl操作都在这个线程当中执行
        mHandler = new Handler(threadLooper);
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                //创建EGL环境 (显示设备、虚拟设备、EGL上下文等)
                mEglBase = new EGLBase(mContext, mWidth, mHeight, mInputSurface, mEglContext);
                //启动编码器
                mMediaCodec.start();
                mIsStart = true;
            }
        });
    }

    /**
     * 进行录制 相当于调用一次就有一个新的图像需要进行编码
     * @param textureId
     * @param timestamp
     */
    public void fireFrame(final int textureId, final long timestamp) {
        if(!mIsStart){
            return;
        }
        //进行录制的时候，要去子线程中进行录制
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                //把图像画到虚拟屏幕
                mEglBase.draw(textureId, timestamp);
                //从编码器的输出缓冲区获取编码后的数据就ok了
                getCodec(false);
            }
        });
    }

    /**
     * 获取编码后 的数据
     * @param endOfStream   标记是否结束录制
     */
    private void getCodec(boolean endOfStream) {
        //不录了， 给mediacodec一个标记
        if (endOfStream) {
            mMediaCodec.signalEndOfInputStream();
        }
        //输出缓冲区
        MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
        while (true){
            //等待10 ms
            int status = mMediaCodec.dequeueOutputBuffer(bufferInfo, 10_000);
            if(status == MediaCodec.INFO_TRY_AGAIN_LATER){  //不断地重试, 表示需要更多的数据才能编码出图像
                // 如果是停止录制，我就继续循环，继续循环就表示不会接收到新的等待编码的图像，
                // 相当于保证MediaCodec中所有采集到的数据也就是待编码的数据都能被编码完成，不断地重试 取出编码器中的编码好的数据
                // 如果标记不是停止，我们退出，下一轮接收到更多数据再来取输出编码后的数据
                if(!endOfStream){
                    break;
                }
            }else if(status == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED){
                //开始编码 就会调用一次
                MediaFormat outputFormat = mMediaCodec.getOutputFormat();
                //配置封装器
                // 增加一路指定格式的媒体流 视频
                mIndex = mMediaMuxer.addTrack(outputFormat);
                mMediaMuxer.start();
            } else if (status == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                //忽略
            } else {
                //成功 取出一个有效的输出图像
                ByteBuffer outputBuffer = mMediaCodec.getOutputBuffer(status);
                //如果获取的ByteBuffer 是配置信息 ,不需要写到mp4
                if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
                    bufferInfo.size = 0;
                }
                if (bufferInfo.size != 0) {
                    //实现 快慢的录制 修改时间戳就可以了
                    bufferInfo.presentationTimeUs = (long) (bufferInfo.presentationTimeUs / mSpeed);
                    //写到mp4
                    //根据偏移定位
                    outputBuffer.position(bufferInfo.offset);
                    //ByteBuffer 可读写总长度
                    outputBuffer.limit(bufferInfo.offset + bufferInfo.size);
                    //写出
                    mMediaMuxer.writeSampleData(mIndex, outputBuffer, bufferInfo);
                }
                //输出缓冲区 我们就使用完了，可以回收了，让mediacodec继续使用
                mMediaCodec.releaseOutputBuffer(status, false);
                //结束
                if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                    break;
                }
            }
        }
    }

    public void stop(){
        mIsStart = false;
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                //不录了，给一个标记停止录制
                getCodec(true);
                mMediaCodec.stop();
                mMediaCodec.release();
                mMediaCodec = null;
                mMediaMuxer.stop();
                mMediaMuxer.release();
                mMediaMuxer = null;
                mEglBase.release();
                mEglBase = null;
                //这句不能加
                //mInputSurface.release();
                mInputSurface = null;
                mHandler.getLooper().quitSafely();
                mHandler = null;
            }
        });
    }

}
