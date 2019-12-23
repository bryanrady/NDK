package com.bryanrady.douyin.widget;

import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

import com.bryanrady.douyin.filter.CameraFilter;
import com.bryanrady.douyin.filter.ScreenFilter;
import com.bryanrady.douyin.util.CameraHelper;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * 自定义GLSurfaceView的渲染器
 */
public class DouYinRenderer implements GLSurfaceView.Renderer, SurfaceTexture.OnFrameAvailableListener {
    private ScreenFilter mScreenFilter;
    private CameraFilter mCameraFilter;
    private DouYinView mDouYinView;
    private CameraHelper mCameraHelper;
    private SurfaceTexture mSurfaceTexture;
    private int[] mTextures;
    private float[] mMtx = new float[16];

    public DouYinRenderer(DouYinView douYinView){
        this.mDouYinView = douYinView;
        //注意：我们使用OpenGL必须在GLThread线程，而这个渲染器的构造方法不是在GLThread线程中
        //mScreenFilter = new ScreenFilter(mDouYinView.getContext());
    }

    /**
     * 画布（图像缓冲区） 画布创建好了 在这里面进行一些初始化操作
     * @param gl
     * @param config
     */
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        //默认使用前置摄像头
        mCameraHelper = new CameraHelper(Camera.CameraInfo.CAMERA_FACING_BACK);
        //通过OpenGL 创建纹理id
        mTextures = new int[1];
        GLES20.glGenTextures(mTextures.length, mTextures, 0);
        //准备好摄像头绘制的画布(通过OpenGL来进行摄像头图像的显示)   参数是一个纹理id
        mSurfaceTexture = new SurfaceTexture(mTextures[0]);
        mSurfaceTexture.setOnFrameAvailableListener(this);
        //用来渲染图像数据
        mScreenFilter = new ScreenFilter(mDouYinView.getContext());
        //用来写到FBO缓存
        mCameraFilter = new CameraFilter(mDouYinView.getContext());
    }

    /**
     * 画布发生改变 从参数拿到改变的宽高
     * @param gl
     * @param width
     * @param height
     */
    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        //开启预览
        mCameraHelper.startPreview(mSurfaceTexture);
        //设置画布大小
        mScreenFilter.onReady(width, height);
        mCameraFilter.onReady(width, height);
    }

    /**
     * 绘制 OpenGl 就是用来绘制的 可以说它就是一个高级画笔
     * @param gl
     */
    @Override
    public void onDrawFrame(GL10 gl) {
        //配置屏幕颜色 告诉OpenGL需要把屏幕的底色清理成什么颜色   黑色
        GLES20.glClearColor(0,0,0,0);
        //调用glClear执行glClearColor配置的屏幕颜色
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);

        /**
         * 把摄像头的采集到的图像数据显示出来
         * 现在数据都存储在mSurfaceTexture里面，我们要想办法把数据从mSurfaceTexture这里面取出来
         */
        //更新纹理 然后我们才能够使用OpenGL从SurfaceTexture当中获得数据 进行渲染
        mSurfaceTexture.updateTexImage();

        //SurfaceTexture 比较特殊，在OpenGL当中 使用的是特殊的采样器 samplerExternalOES （而不是sampler2D）
        //获得变换矩阵 必须要通过变换矩阵来变换一下采样的坐标 才能得到正确的采样坐标点
        mSurfaceTexture.getTransformMatrix(mMtx);

        //直接显示到屏幕上
        //mScreenFilter.onDrawFrame2(mTextures[0], mMtx);

        //先写到FBO中，再从FBO中取出数据写到屏幕上
        //不需要显示到屏幕上 负责写入到FBO(帧缓存)
        mCameraFilter.setMatrix(mMtx);
        //返回的是fbo的纹理Id,然后将fbo的纹理id交给ScreenFilter进行绘制，实际上就是交给了父类
        //责任链 各司其职  就像流水线一样
        int textureId = mCameraFilter.onDrawFrame(mTextures[0]);
        //加效果滤镜
        // textureId  = 效果1.onDrawFrame(textureId);
        // textureId = 效果2.onDrawFrame(textureId);
        //....
        //加完效果之后再显示到屏幕中去
        mScreenFilter.onDrawFrame(textureId);
    }

    /**
     * 在SurfaceTexture有一个有效的新数据的时候回调
     * @param surfaceTexture
     */
    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        //有一个新的图像后,通过调用requestRender就请求GLThread 回调一次 onDrawFrame()
        mDouYinView.requestRender();
    }

    /**
     * 当应用处于后台的时候 就关闭摄像头
     */
    public void onSurfaceDestroyed(){
        mCameraHelper.stopPreview();
    }

    public void startRecord(float speed) {
    }

    public void stopRecord() {
    }
}
