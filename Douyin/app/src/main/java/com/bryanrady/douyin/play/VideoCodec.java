package com.bryanrady.douyin.play;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.util.Log;

import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * 解码录制完成后的数据
 */
public class VideoCodec {

    private ISurface mISurface;
    private String mDataSource;
    private MediaExtractor mMediaExtractor;
    private int mWidth;
    private int mHeight;
    //录制的时候给的默认值
    private int mBitrate = 1500_000;
    private int mFps = 20;
    private MediaCodec mDecodeMediaCodec;
    //是否正在解码
    private boolean mIsDecoding;
    private byte[] mOutData;
    private DecodeThread mDecodeThread;

    public void setDisplay(ISurface surface){
        mISurface = surface;
    }

    /**
     * 设置要解码的资源文件 也就是文件地址
     * @param dataSource
     */
    public void setDataSource(String dataSource){
        mDataSource = dataSource;
    }

    /**
     * 准备解码的工作
     */
    public void prepare(){
        //解封装器（解复用器）
        mMediaExtractor = new MediaExtractor();
        try {
            //把视频交给解复用器
            mMediaExtractor.setDataSource(mDataSource);
        } catch (IOException e) {
            e.printStackTrace();
        }
        int videoIndex = -1;
        MediaFormat videoFormat = null;
        //得到各种流  mp4文件 1路音频 1路视频
        int trackCount = mMediaExtractor.getTrackCount();
        for (int i = 0; i < trackCount; i++){
            //获得这路流的格式
            MediaFormat trackFormat = mMediaExtractor.getTrackFormat(i);
            String mime = trackFormat.getString(MediaFormat.KEY_MIME);
            // video/  audio/
            if(mime.startsWith("video/")){
                videoIndex = i;
                videoFormat = trackFormat;
                break;
            }
        }
        if(videoFormat != null){
            //解码 videoIndex 这一路流
            mWidth = videoFormat.getInteger(MediaFormat.KEY_WIDTH);
            mHeight = videoFormat.getInteger(MediaFormat.KEY_HEIGHT);
            //有可能视频流没有包含码率和帧率
            if(videoFormat.containsKey(MediaFormat.KEY_BIT_RATE)){
                mBitrate = videoFormat.getInteger(MediaFormat.KEY_BIT_RATE);
            }
            if(videoFormat.containsKey(MediaFormat.KEY_FRAME_RATE)){
                mFps = videoFormat.getInteger(MediaFormat.KEY_FRAME_RATE);
            }
            // 有个别手机  如小米(x型号) 解码出来的数据格式不是yuv420p
            // 所以这里设置 解码数据格式 指定为yuv420
            videoFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar);

            try {
                //创建这个类型的解码器
                mDecodeMediaCodec = MediaCodec.createDecoderByType(videoFormat.getString(MediaFormat.KEY_MIME));
                mDecodeMediaCodec.configure(videoFormat,null,null,0);
            } catch (IOException e) {
                e.printStackTrace();
            }

            //选择流 后续读取这个流
            mMediaExtractor.selectTrack(videoIndex);

            //将视频流的信息回调出去
            if (mISurface != null){
                mISurface.getVideoParameters(mWidth,mHeight,mBitrate,mFps);
            }
        }
    }

    /**
     * 开始解码
     */
    public void start(){
        mIsDecoding = true;
        //接收 解码后的数据 yuv数据大小是 w*h*3/2
        mOutData = new byte[mWidth * mHeight * 3 / 2];
        mDecodeThread = new DecodeThread();
        mDecodeThread.start();
    }

    public void stop(){
        mIsDecoding = false;
        if(mDecodeThread != null && mDecodeThread.isAlive()){
            try {
                //等待3s
                mDecodeThread.join(3_000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            //如果3s后线程还没结束
            if (mDecodeThread.isAlive()){
                //中断掉
                mDecodeThread.interrupt();
            }
            mDecodeThread = null;
        }
    }

    /**
     * 解码线程
     */
    private class DecodeThread extends Thread{

        @Override
        public void run() {
            if(mDecodeMediaCodec == null){
                return;
            }
            //开启解码器
            mDecodeMediaCodec.start();

            boolean isEOF = false;
            MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
            //判断线程有没有中断
            while (!isInterrupted()){
                if(!mIsDecoding){
                    break;
                }
                // 如果 eof是true 就表示读完了，就不执行putBuffer2Codec方法了
                // 但是不代表解码完了
                if (!isEOF) {
                    isEOF = putBuffer2Codec();
                }
                //从输出缓冲区获取数据(解码之后的数据)
                int index = mDecodeMediaCodec.dequeueOutputBuffer(bufferInfo, 100);
                if(index >= 0){
                    //表示获取到一个有效的输出缓冲区，就是能够获取到了解码后的数据
                    ByteBuffer outputBuffer = mDecodeMediaCodec.getOutputBuffer(index);
                    //作一个容错判断
                    //p30的手机发现 bufferInfo.size  491520  mOutData.length 460800 不相等
                    if (bufferInfo.size >= mOutData.length){
                        //从输出缓冲区中取出数据 存到outData yuv420
                        outputBuffer.get(mOutData);
                        if (mISurface != null){
                            //把数据加入到队列中，然后从队列中取出数据交给OpenGL进行绘制
                            mISurface.offer(mOutData);
                        }

                        //释放掉这个输出缓冲区 释放
                        mDecodeMediaCodec.releaseOutputBuffer(index,false);
                    }
                }
                //结束，全部解码完成了
                if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0){
                    break;
                }
            }

            mDecodeMediaCodec.stop();
            mDecodeMediaCodec.release();
            mDecodeMediaCodec = null;
            mMediaExtractor.release();
            mMediaExtractor = null;
        }

        /**
         * 把数据加入解码器
         * @return true : 没有更多数据，表示读完了
         *         false: 还有数据没有读完
         */
        private boolean putBuffer2Codec() {
            // 输入缓冲区的下标   如果是 -1 就一直等待
            int index = mDecodeMediaCodec.dequeueInputBuffer(100);
            //如果下标 index >= 0  就表示拿到的输入缓冲区是有效的
            if (index >= 0 ){
                //通过index拿到有效的输入缓冲区
                ByteBuffer inputBuffer = mDecodeMediaCodec.getInputBuffer(index);
                //清理一下脏数据
                inputBuffer.clear();
                //从解封装器中取数据加入到ByteBuffer中
                // 读数据存入 ByteBuffer 从第0个开始存
                int size =  mMediaExtractor.readSampleData(inputBuffer, 0);
                if(size < 0){
                    //表示没读到数据 可能已经读完了，表示没有数据可读了
                    //MediaCodec.BUFFER_FLAG_END_OF_STREAM  给个标记 表示没有更多数据可以从输出缓冲区获取了，也就是解码完了
                    mDecodeMediaCodec.queueInputBuffer(index,0,0,0,
                            MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                    return true;
                }else{
                    //把塞了数据的输入缓冲区塞回去，然后才能够执行解码
                    mDecodeMediaCodec.queueInputBuffer(index,0, size, mMediaExtractor.getSampleTime(), 0);
                    //丢掉已经加入解码器的数据 （不丢就会读重复的数据）
                    mMediaExtractor.advance();
                    return false;
                }
            }
            return false;
        }
    }



}
