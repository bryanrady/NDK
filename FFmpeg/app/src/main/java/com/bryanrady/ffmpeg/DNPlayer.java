package com.bryanrady.ffmpeg;

import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

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
        mSurfaceHolder = surfaceView.getHolder();
        mSurfaceHolder.addCallback(this);
    }

    public void startPlay(){

    }

    public void stopPlay(){

    }

    /**
     * 准备好要播放的视频 实现视频的解封装调用native方法
     */
    public void prepare() {
        native_prepare(mDataSource);
    }

    public void release(){
        mSurfaceHolder.removeCallback(this);
    }

    private void onError(int errorCode){
        Log.e("wanqgingbin","成功调用了Java的onError");
        if (mOnPreparedListener!=null){
            mOnPreparedListener.onError(errorCode);
        }
    }

    private void onPrepared(){
        Log.e("wanqgingbin","onPrepared");
        if (mOnPreparedListener!=null){
            mOnPreparedListener.onPrepared();
        }
    }

    public interface OnPreparedListener{
        void onPrepared();
        void onError(int errorCode);
    }

    public void setOnPreparedListener(OnPreparedListener preparedListener) {
        this.mOnPreparedListener = preparedListener;
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
}
