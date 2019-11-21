package com.bryanrady.rtmp;

import android.app.Activity;
import android.view.SurfaceHolder;

public class LivePusher {

    static {
        System.loadLibrary("native-lib");
    }

    private AudioChannel mAudioChannel;
    private VideoChannel mVideoChannel;

    /**
     * @param activity
     * @param width     摄像头采集数据的宽
     * @param height    摄像头采集数据的高
     * @param bitrate   码率  跟我们编码之后的数据量大小是成正比的 码率越高编码出来的数据量越高，压缩出来的图像就越清晰，但是也有一个上限值
     * @param fps       帧率  图像刷新率
     * @param cameraId  默认使用的摄像头 前置还是后置
     */
    public LivePusher(Activity activity, int width, int height, int bitrate, int fps, int cameraId) {
        native_init();
        mVideoChannel = new VideoChannel(this, activity, width, height, bitrate, fps, cameraId);
        mAudioChannel = new AudioChannel();
    }

    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {
        mVideoChannel.setPreviewDisplay(surfaceHolder);
    }

    public void switchCamera() {
        mVideoChannel.switchCamera();
    }

    public void startLive(String path) {
        //开始直播，进行连接服务器操作
        native_start(path);
        mVideoChannel.startLive();
        mAudioChannel.startLive();
    }

    public void stopLive(){
        mVideoChannel.stopLive();
        mAudioChannel.stopLive();
        native_stop();
    }

    /**
     * 进行一些初始化
     */
    public native void native_init();

    /**
     * 创建编码器
     * @param width
     * @param height
     * @param fps
     * @param bitrate
     */
    public native void native_setVideoEncInfo(int width, int height, int fps, int bitrate);

    /**
     * 启动一个线程进行Tcp连接到服务器 并开始推流
     * @param path
     */
    public native void native_start(String path);

    /**
     * 添加视频数据到队列中去
     * @param data
     */
    public native void native_pushVideo(byte[] data);

    /**
     * 停止推流
     */
    public native void native_stop();

    public native void native_release();

}
