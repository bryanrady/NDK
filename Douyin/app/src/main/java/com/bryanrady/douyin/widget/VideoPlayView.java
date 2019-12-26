package com.bryanrady.douyin.widget;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;

import com.bryanrady.douyin.play.ISurface;
import com.bryanrady.douyin.play.VideoCodec;
import com.bryanrady.douyin.filter.SoulFilter;

import java.util.LinkedList;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class VideoPlayView extends GLSurfaceView implements GLSurfaceView.Renderer, ISurface {

    private VideoCodec mVideoCodec;
    private int mWidth;
    private int mHeight;
    private int mBitrate;
    private int mFps;
    private int mInterval;
    private LinkedList<byte[]> mQueue;
    private SoulFilter mSoulFilter;
    private long mLastRenderTime;

    public VideoPlayView(Context context) {
        this(context, null);
    }

    public VideoPlayView(Context context, AttributeSet attrs) {
        super(context, attrs);
        /**
         * 配置GLSurfaceView
         */
        //设置EGL版本
        setEGLContextClientVersion(2);
        setRenderer(this);
        //设置渲染模式
        //按需渲染   RENDERMODE_WHEN_DIRTY     就是手动调用 requestRender 请求GLThread 回调一次 onDrawFrame
        //连续渲染   RENDERMODE_CONTINUOUSLY   就是自动的回调 onDrawFrame
        setRenderMode(RENDERMODE_CONTINUOUSLY);

        mQueue = new LinkedList<>();

        mVideoCodec = new VideoCodec();
        mVideoCodec.setDisplay(this);
    }

    public void setDataSource(String dataSource){
        mVideoCodec.setDataSource(dataSource);
    }

    public void startPlay(){
        mVideoCodec.prepare();
        mVideoCodec.start();
    }

    public void stopPlay(){
        mVideoCodec.stop();
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        mSoulFilter = new SoulFilter(getContext());
        mSoulFilter.onReady2(mWidth, mHeight, mFps);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES20.glViewport(0,0, width, height);
    }

    /**
     * 绘制
     * 16ms  vSync: 垂直同步信号  16ms调用一次onDrawFrame
     * 如果视频达不到 60fps 那么就不对了 就会变成快进效果了
     * @param gl
     */
    @Override
    public void onDrawFrame(GL10 gl) {
        //这一次渲染时间距离上一次的时间
        long diff = System.currentTimeMillis() - mLastRenderTime;
        //如果不满足 fps算出的时间 就sleep
        long delay = mInterval - diff;
        if (delay > 0){
            try {
                Thread.sleep(delay);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        //清理屏幕
        GLES20.glClearColor(0,0,0,0);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);

        //取出yuv数据
        byte[] yuv = poll();
        if (yuv != null){
            mSoulFilter.onDrawFrame(yuv);
        }
        mLastRenderTime = System.currentTimeMillis();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        super.surfaceDestroyed(holder);
        stopPlay();
    }

    @Override
    public void offer(byte[] data) {
        synchronized (this){
            //这个data一直在改变，所以我们不重新用一个数组来接收的话，会导致数据永远是同一个
            byte[] yuv = new byte[data.length];
            System.arraycopy(data,0, yuv,0, yuv.length);
            mQueue.offer(yuv);
        }
    }

    @Override
    public byte[] poll() {
        synchronized (this){
            return mQueue.poll();
        }
    }

    @Override
    public void getVideoParameters(int width, int height, int bitrate, int fps) {
        mWidth = width;
        mHeight = height;
        mBitrate = bitrate;
        mFps = fps;
        mInterval = 1000/mFps;
        if (mSoulFilter != null){
            mSoulFilter.onReady2(mWidth,mHeight,mFps);
        }
    }
}
