package com.bryanrady.rtmp;

import android.app.Activity;
import android.hardware.Camera;
import android.view.SurfaceHolder;

/**
 * 用来操作Camera 创建Camera
 */
public class VideoChannel implements Camera.PreviewCallback, CameraHelper.OnChangedSizeListener {

    private LivePusher mLivePusher;
    private CameraHelper mCameraHelper;
    private int mBitRate;
    private int mFps;
    private boolean mIsLiving;

    public VideoChannel(LivePusher livePusher, Activity activity, int width, int height, int bitrate, int fps, int cameraId) {
        mLivePusher = livePusher;
        mBitRate = bitrate;
        mFps = fps;
        mCameraHelper = new CameraHelper(activity, cameraId, width, height);
        //回调 得到摄像头采集的数据
        mCameraHelper.setPreviewCallback(this);
        //设置回调 获得真实的摄像头数据宽、高
        mCameraHelper.setOnChangedSizeListener(this);
    }

    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {
        mCameraHelper.setPreviewDisplay(surfaceHolder);
    }

    /**
     * 得到NV21数据 已经旋转好的 采集到的数据就是一张图片
     * @param data  这个数组就是摄像头采集到的数据 数据格式是 NV21
     * @param camera
     */
    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        if (mIsLiving) {
            mLivePusher.native_pushVideo(data);
        }
    }

    public void switchCamera() {
        mCameraHelper.switchCamera();
    }

    /**
     * 真实摄像头数据的宽、高
     * @param w
     * @param h
     */
    @Override
    public void onChanged(int w, int h) {
        //初始化编码器
        mLivePusher.native_setVideoEncInfo(w, h, mFps, mBitRate);
    }

    public void startLive() {
        mIsLiving = true;
    }

    public void stopLive() {
        mIsLiving = false;
    }
}
