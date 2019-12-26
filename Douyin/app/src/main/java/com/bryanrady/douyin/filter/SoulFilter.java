package com.bryanrady.douyin.filter;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.Matrix;
import android.util.Log;

import com.bryanrady.douyin.R;
import com.bryanrady.douyin.util.GLImage;
import com.bryanrady.douyin.util.OpenGLUtils;

/**
 * 灵魂出窍
 */
public class SoulFilter extends AbstractFilter {

    //肉体
    private GLImage mBodyImage;
    //灵魂
    private GLImage mSoulImage;
    private int[] mTextures;
    private int mAlpha;
    private int mSamplerV;
    private int mSamplerU;
    private int mSamplerY;
    private int mFps;
    private int mInterval;
    private float[] mMatrix = new float[16];

    public SoulFilter(Context context) {
        super(context.getApplicationContext(), R.raw.soul_vertex_shader, R.raw.soul_fragment_shader);
        mBodyImage = new GLImage();
        mSoulImage = new GLImage();
        mSamplerY = GLES20.glGetUniformLocation(mGLProgramId, "sampler_y");
        mSamplerU = GLES20.glGetUniformLocation(mGLProgramId,"sampler_u");
        mSamplerV = GLES20.glGetUniformLocation(mGLProgramId,"sampler_v");
        mAlpha = GLES20.glGetUniformLocation(mGLProgramId, "alpha");

        //创建纹理 3个纹理 yuv
        mTextures = new int[3];
        OpenGLUtils.glGenTextures(mTextures);
    }

    public void onReady2(int width,int height,int fps){
        super.onReady(width,height);
        mFps = fps;
        mBodyImage.initSize(width, height);
        mSoulImage.initSize(width, height);
    }

    public void onDrawFrame(byte[] yuv) {
        //把yuv分离出来 保存在 image中的 y、u、v三个变量中
        mBodyImage.initData(yuv);
        //判断分离出的数据是否有效
        if (!mBodyImage.hasImage()){
            return;
        }
        //启用着色器程序
        GLES20.glUseProgram(mGLProgramId);
        //初始化矩阵 不进行任何缩放平移
        Matrix.setIdentityM(mMatrix,0);
        //给肉体的 无变化矩阵
        GLES20.glUniformMatrix4fv(mVMatrix,1,false, mMatrix,0);
        //透明度 肉体不透明
        GLES20.glUniform1f(mAlpha,1);
        //传值
        //画画
        onDrawBody(mBodyImage);
        //混合灵魂
        onBlendSoul(yuv);
    }

    private void onDrawBody(GLImage image){
        //传递坐标
        mGLVertexBuffer.position(0);
        GLES20.glVertexAttribPointer(mVPosition, 2, GLES20.GL_FLOAT, false, 0, mGLVertexBuffer);
        GLES20.glEnableVertexAttribArray(mVPosition);

        mGLTextureBuffer.position(0);
        GLES20.glVertexAttribPointer(mVCoord, 2, GLES20.GL_FLOAT, false, 0, mGLTextureBuffer);
        GLES20.glEnableVertexAttribArray(mVCoord);

        //传递yuv数据
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTextures[0]);
        //把y数据与 0纹理绑定
        //  GL_LUMINANCE: yuv 给这个
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D,0, GLES20.GL_LUMINANCE,
                mOutputWidth, mOutputHeight,0, GLES20.GL_LUMINANCE,
                GLES20.GL_UNSIGNED_BYTE, image.getY());
        GLES20.glUniform1i(mSamplerY, 0);

        //u数据
        GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTextures[1]);
        //  GL_LUMINANCE: yuv 给这个
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D,0, GLES20.GL_LUMINANCE,
                mOutputWidth/2,mOutputHeight/2,0, GLES20.GL_LUMINANCE,
                GLES20.GL_UNSIGNED_BYTE, image.getU());
        GLES20.glUniform1i(mSamplerU, 1);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTextures[2]);
        //  GL_LUMINANCE: yuv 给这个
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D,0, GLES20.GL_LUMINANCE,
                mOutputWidth/2,mOutputHeight/2,0, GLES20.GL_LUMINANCE,
                GLES20.GL_UNSIGNED_BYTE, image.getV());
        GLES20.glUniform1i(mSamplerV, 2);

        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);

        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
    }

    private void onBlendSoul(byte[] yuv){
        mInterval++;
        // 没保存一个灵魂 或者使用次数已经达到上限了
        // 灵魂只能使用x次 使用完了之后就要更新灵魂, 不然的话灵魂永远是一个图像
        if (!mSoulImage.hasImage() || mInterval > mFps){
            //次数重置为1
            mInterval = 1;
            //记录新灵魂
            mSoulImage.initData(yuv);
        }
        if (!mSoulImage.hasImage()){
            return;
        }

        //开启混合模式
        GLES20.glEnable(GLES20.GL_BLEND);
        //1：源 灵魂  GL_ONE:画灵魂自己
        //2: 肉体  也是肉体自己
        //两个都是用自己原本的颜色去混合
//        GLES20.glBlendFunc(GLES20.GL_ONE,GLES20.GL_ONE);
        //让灵魂整体颜色变淡
        // GL_SRC_ALPHA： 取源(灵魂)的alpha 作为因子
        // 假设alpha是0.2 rgb都是1 -> 混合就是用 rgb都是 0.2*1 整体变淡
        GLES20.glBlendFunc(GLES20.GL_SRC_ALPHA, GLES20.GL_ONE);

        //初始化矩阵 不进行任何缩放平移
        Matrix.setIdentityM(mMatrix,0);
        //设置缩放大小 本次放大为 1+当前灵魂次数占总次数*2的比例
        //不一次放太大 为了达到较好的表现效果 fps*2
        //所以这里值为 1+1/60 ---> 1+20/40 1.025... ---> 1.5
        float scale = 1.0f + mInterval / (mFps * 2.f);
        Matrix.scaleM(mMatrix,0,scale,scale,0);
        //给肉体的 无变化矩阵
        GLES20.glUniformMatrix4fv(mVMatrix,1,false, mMatrix,0);

        //传递透明度 透明度值为0-1 渐渐降低 0.1+x/100 x为fps-[0~fps]
        //这里值为0.29 ---> 0.1
        GLES20.glUniform1f(mAlpha, 0.1f + (mFps - mInterval) / 100.f);

        //画灵魂 画肉体是一个方法 所以调用这个方法传灵魂的图像进去就行
        onDrawBody(mSoulImage);
    }
}
