package com.bryanrady.douyin.filter;

import android.content.Context;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;

import com.bryanrady.douyin.R;
import com.bryanrady.douyin.util.OpenGLUtils;

/**
 * 使用FBO的Camera滤镜
 */
public class CameraFilter extends AbstractFilter {

    private int[] mFrameBuffers;
    private int[] mFrameBufferTextures;
    private float[] mMtx;

    public CameraFilter(Context context) {
        super(context, R.raw.camera_vertex_shader, R.raw.camera_fragment_shader);
    }

    /**
     * 修改坐标  我们这里来修改纹理坐标
     */
    @Override
    protected void changeCoordinate() {
        super.changeCoordinate();
        mGLTextureBuffer.clear();
        //不需要显示到屏幕上 根据纹理坐标来传递数据
        //摄像头是颠倒的 所以我们要来进行旋转
//        float[] texture = {
//                0.0f, 0.0f,
//                1.0f, 0.0f,
//                0.0f, 1.0f,
//                1.0f, 1.0f
//        };
        //镜像
//        float[] texture = {
//                1.0f, 0.0f,
//                0.0f, 0.0f,
//                1.0f, 1.0f,
//                0.0f, 1.0f,
//        };
        //修复旋转 逆时针旋转90度
        float[] texture = {
                0.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 0.0f,
                1.0f, 1.0f,
        };
        mGLTextureBuffer.put(texture);
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
    public int onDrawFrame(int textureId) {
        //把父类的删掉 自己重写
        //return super.onDrawFrame(textureId);

        //设置显示窗口
        GLES20.glViewport(0, 0, mOutputWidth, mOutputHeight);

        //不调用的话就是默认的操作glsurfaceview中的纹理了。显示到屏幕上了
        //这里我们还只是把它画到fbo中(缓存)
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER,mFrameBuffers[0]);

        //使用着色器
        GLES20.glUseProgram(mGLProgramId);

        //传递坐标
        mGLVertexBuffer.position(0);
        GLES20.glVertexAttribPointer(mVPosition, 2, GLES20.GL_FLOAT, false, 0, mGLVertexBuffer);
        GLES20.glEnableVertexAttribArray(mVPosition);

        mGLTextureBuffer.position(0);
        GLES20.glVertexAttribPointer(mVCoord, 2, GLES20.GL_FLOAT, false, 0, mGLTextureBuffer);
        GLES20.glEnableVertexAttribArray(mVCoord);

        //变换矩阵
        GLES20.glUniformMatrix4fv(mVMatrix,1,false, mMtx,0);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        //因为这一层是摄像头后的第一层，所以需要使用扩展的  GL_TEXTURE_EXTERNAL_OES
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId);
        GLES20.glUniform1i(mVTexture, 0);

        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);

        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,0);
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER,0);
        //返回fbo的纹理id
        return mFrameBufferTextures[0];
    }

    @Override
    public void release() {
        super.release();
        destroyFrameBuffers();
    }

    public void setMatrix(float[] mtx) {
        this.mMtx = mtx;
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
