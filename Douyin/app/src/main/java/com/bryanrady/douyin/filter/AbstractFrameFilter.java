package com.bryanrady.douyin.filter;

import android.content.Context;
import android.opengl.GLES20;

import com.bryanrady.douyin.util.OpenGLUtils;

public class AbstractFrameFilter extends AbstractFilter{

    protected int[] mFrameBuffers;
    protected int[] mFrameBufferTextures;

    public AbstractFrameFilter(Context context, int vertexShaderId, int fragmentShaderId) {
        super(context, vertexShaderId, fragmentShaderId);
    }

    @Override
    public void onReady(int width, int height) {
        super.onReady(width, height);
        if (mFrameBuffers != null) {
            destroyFrameBuffers();
        }
        /**
         * 流程： 我们先把摄像头的数据写入到FBO帧缓存中，然后再从帧缓存中取出数据显示到屏幕上
         */
        //FBO(Frame Buffer Object)也要在GLThread线程中创建

        //1.创建一个FBO缓存(当作一个离屏屏幕，就是不展示的屏幕，我们创建一个属于这个屏幕的纹理)
        mFrameBuffers = new int[1];
        // 1: 创建几个FBO  2: 保存fbo id的数据  3:从这个数组的第几个开始保存
        GLES20.glGenFramebuffers(mFrameBuffers.length, mFrameBuffers, 0);

        //2.创建属于FBO的纹理 mFrameBufferTextures  用来记录纹理id
        mFrameBufferTextures = new int[1];
        //这个纹理创建出来不能够直接拿去使用，我们之前创建的纹理直接可以拿去使用(当然也可以进行配置)，
        //但是在FBO中，我们创建的纹理不能够直接拿去使用，必须要来配置一下
        //创建纹理并配置
        OpenGLUtils.glGenTextures(mFrameBufferTextures);

        //3.让FBO与FBO的纹理产生联系
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mFrameBufferTextures[0]);
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D,0, GLES20.GL_RGBA, mOutputWidth, mOutputHeight,
                0, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, null);
        //让fbo与纹理绑定起来，后续的操作就是在操作fbo与这个纹理了
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, mFrameBuffers[0]);
        GLES20.glFramebufferTexture2D(GLES20.GL_FRAMEBUFFER, GLES20.GL_COLOR_ATTACHMENT0,
                GLES20.GL_TEXTURE_2D, mFrameBufferTextures[0], 0);
        //解绑
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D,0);
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER,0);
    }

    @Override
    public void release() {
        super.release();
        destroyFrameBuffers();
    }

    private void destroyFrameBuffers() {
        //删除fbo的纹理
        if (mFrameBufferTextures != null) {
            GLES20.glDeleteTextures(1, mFrameBufferTextures, 0);
            mFrameBufferTextures = null;
        }
        //删除fbo
        if (mFrameBuffers != null) {
            GLES20.glDeleteFramebuffers(1, mFrameBuffers, 0);
            mFrameBuffers = null;
        }
    }
}
