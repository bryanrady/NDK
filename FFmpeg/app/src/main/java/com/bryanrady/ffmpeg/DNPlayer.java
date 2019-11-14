package com.bryanrady.ffmpeg;

import android.os.Looper;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.Toast;

/**
 * 提供Java层播放器功能 播放与停止
 * @author: wangqingbin
 * @date: 2019/11/9 12:51
 */
public class DNPlayer implements SurfaceHolder.Callback {

    static {
        System.loadLibrary("native-lib");
    }

    private String mDataSource;
    private SurfaceHolder mSurfaceHolder;
    private OnPreparedListener mOnPreparedListener;
    private OnErrorListener mOnErrorListener;
    private OnProgressListener mOnProgressListener;

    /**
     * 设置播放的文件路径 或者 直播地址
     * @param dataSource
     */
    public void setDataSource(String dataSource){
        this.mDataSource = dataSource;
    }

    /**
     * 设置播放显示的画布
     * @param surfaceView
     */
    public void setSurfaceView(SurfaceView surfaceView){
        if(mSurfaceHolder != null){
            mSurfaceHolder.removeCallback(this);
        }
        mSurfaceHolder = surfaceView.getHolder();
        native_set_surface(mSurfaceHolder.getSurface());
        mSurfaceHolder.addCallback(this);
    }

    /**
     * 准备好要播放的视频 实现视频的解封装调用native方法
     */
    public void prepare() {
        native_prepare(mDataSource);
    }

    public void startPlay(){
        native_start();
    }

    public void stopPlay(){
        native_stop();
    }

    public void release(){
        if (mSurfaceHolder != null){
            mSurfaceHolder.removeCallback(this);
        }
        native_release();
    }

    public int getDuration() {
        return native_get_duration();
    }

    public void seek(final int progress) {
        new Thread() {
            @Override
            public void run() {
                native_seek(progress);
            }
        }.start();
    }

    public void pause(){
        new Thread(new Runnable() {
            @Override
            public void run() {
                native_pause();
            }
        }).start();
    }

    private void onError(int errorCode){
        if (mOnErrorListener!=null){
            mOnErrorListener.onError(errorCode);
        }
    }

    private void onPrepared(){
        if (mOnPreparedListener!=null){
            mOnPreparedListener.onPrepared();
        }
    }

    public void onProgress(int progress) {
        if (mOnProgressListener != null) {
            mOnProgressListener.onProgress(progress);
        }
    }

    public interface OnPreparedListener{
        void onPrepared();
    }

    public interface OnErrorListener{
        void onError(int errorCode);
    }

    public interface OnProgressListener {
        void onProgress(int progress);
    }

    public void setOnPreparedListener(OnPreparedListener preparedListener) {
        this.mOnPreparedListener = preparedListener;
    }

    public void setOnErrorListener(OnErrorListener errorListener) {
        this.mOnErrorListener = errorListener;
    }

    public void setOnProgressListener(OnProgressListener progressListener) {
        this.mOnProgressListener = progressListener;
    }

    /**
     * 画布已经创建好了
     * @param holder
     */
    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    /**
     * 画布发生改变 横盘竖屏切换和点击Home键 都会回调这个函数
     * @param holder
     * @param format
     * @param width
     * @param height
     */
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        //画布发生改变了，我们就要把改变的画布传给native
        if(Looper.myLooper() == Looper.getMainLooper()){    //如果当前线程是主线程,则进行UI操作
            native_set_surface(holder.getSurface());
        }
    }

    /**
     * 画布被销毁 按了Home或者退出很多情况
     * @param holder
     */
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    /**
     * 对要播放的视频地址进行解封装 获取视频参数
     * @param dataSource
     */
    private native void native_prepare(String dataSource);

    /**
     * 开始进行音视频的解码与播放
     */
    private native void native_start();

    private native void native_set_surface(Surface surface);

    private native void native_stop();

    private native void native_release();

    private native int native_get_duration();

    //拖动进度条
    private native void native_seek(int progress);

    //暂停
    private native void native_pause();
}
