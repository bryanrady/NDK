package com.bryanrady.douyin.widget;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.SurfaceHolder;

/**
 * GLSurfaceView 集成至 SurfaceView  它内嵌的Surface专门负责OpenGL的渲染
 *      管理Surface 与 EGL
 *      允许自定义渲染器 (Renderer) 这个渲染器是一个接口，我们集成这个接口自己来进行实现渲染器
 *      让渲染器在独立的线程GLThread中运作，和UI线程分离
 *
 *      支持按需渲染和连续渲染
 */
public class DouYinView extends GLSurfaceView {

    public enum Speed {
        MODE_EXTRA_SLOW, MODE_SLOW, MODE_NORMAL, MODE_FAST, MODE_EXTRA_FAST
    }

    //默认正常速度
    private Speed RECORD_SPEED = Speed.MODE_NORMAL;

    public DouYinRenderer mDouYinRenderer;

    public DouYinView(Context context) {
        this(context,null);
    }

    public DouYinView(Context context, AttributeSet attrs) {
        super(context, attrs);
        /**
         * 配置GLSurfaceView
         */
        //设置EGL版本
        setEGLContextClientVersion(2);
        //设置渲染器
        mDouYinRenderer = new DouYinRenderer(this);
        setRenderer(mDouYinRenderer);
        //设置渲染模式
        //按需渲染   RENDERMODE_WHEN_DIRTY     就是手动调用 requestRender 请求GLThread 回调一次 onDrawFrame
        //连续渲染   RENDERMODE_CONTINUOUSLY   就是自动的回调 onDrawFrame
        setRenderMode(RENDERMODE_CONTINUOUSLY);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        super.surfaceDestroyed(holder);
        mDouYinRenderer.onSurfaceDestroyed();
    }

    public void setSpeed(Speed speed){
        this.RECORD_SPEED = speed;
    }

    public void startRecord() {
        float speed = 1.f;
        switch (RECORD_SPEED) {
            case MODE_EXTRA_SLOW:
                speed = 0.3f;
                break;
            case MODE_SLOW:
                speed = 0.5f;
                break;
            case MODE_NORMAL:
                speed = 1.f;
                break;
            case MODE_FAST:
                speed = 1.5f;
                break;
            case MODE_EXTRA_FAST:
                speed = 3.f;
                break;
        }
        mDouYinRenderer.startRecord(speed);
    }

    public void stopRecord() {
        mDouYinRenderer.stopRecord();
    }

    public void enableBigEye(boolean isChecked) {
        mDouYinRenderer.enableBigEye(isChecked);
    }

    public void enableSticker(boolean isChecked) {
        mDouYinRenderer.enableSticker(isChecked);
    }

    public void enableBeauty(boolean isChecked) {
        mDouYinRenderer.enableBeauty(isChecked);
    }

}
