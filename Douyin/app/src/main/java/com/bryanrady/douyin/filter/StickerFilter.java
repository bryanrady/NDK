package com.bryanrady.douyin.filter;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLUtils;

import com.bryanrady.douyin.R;
import com.bryanrady.douyin.face.Face;
import com.bryanrady.douyin.util.OpenGLUtils;

/**
 * 贴纸
 */
public class StickerFilter extends AbstractFrameFilter {

    private Bitmap mStickerBitmap;
    private int[] mBitmapTextures;
    private Face mFace;

    public StickerFilter(Context context) {
        super(context.getApplicationContext(), R.raw.base_vertex_shader, R.raw.base_fragment_shader);
        mStickerBitmap = BitmapFactory.decodeResource(context.getApplicationContext().getResources(), R.drawable.erduo_000,null);
    }

    public void setFace(Face face){
        mFace = face;
    }

    @Override
    public void onReady(int width, int height) {
        super.onReady(width, height);
        mBitmapTextures = new int[1];
        OpenGLUtils.glGenTextures(mBitmapTextures);
        //表示后续的操作 就是作用于这个纹理上
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mBitmapTextures[0]);
        //将Bitmap与Bitmap的纹理id发生关系
        GLUtils.texImage2D(GLES20.GL_TEXTURE_2D,0, mStickerBitmap, 0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
    }

    @Override
    public int onDrawFrame(int textureId) {
        if (mFace == null){
            return textureId;
        }
        //设置显示窗口
        GLES20.glViewport(0, 0, mOutputWidth, mOutputHeight);
        //不调用的话就是默认的操作GLSurfaceView中的纹理了,显示到屏幕上了
        //这里我们还只是把它画到fbo中(缓存)
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, mFrameBuffers[0]);
        //使用着色器程序
        GLES20.glUseProgram(mGLProgramId);
        //传递顶点坐标
        mGLVertexBuffer.position(0);
        GLES20.glVertexAttribPointer(mVPosition,2, GLES20.GL_FLOAT,false,0, mGLVertexBuffer);
        GLES20.glEnableVertexAttribArray(mVPosition);
        //传递纹理坐标
        mGLTextureBuffer.position(0);
        GLES20.glVertexAttribPointer(mVCoord,2, GLES20.GL_FLOAT, false,0, mGLTextureBuffer);
        GLES20.glEnableVertexAttribArray(mVCoord);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId);
        GLES20.glUniform1i(mVTexture, 0);

        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);

        //解绑
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);

        //加上贴纸
        onDrawStick();

        //返回fbo的纹理id
        return mFrameBufferTextures[0];
    }

    /**
     * 将贴纸画上去
     */
    private void onDrawStick() {
        //开启混合    将多张图片进行混合
        GLES20.glEnable(GLES20.GL_BLEND);
        //设置贴图模式
        // 1：src : 源图因子 ： 要画的是源  (耳朵)
        // 2: dst : 已经画好的是目标  (从其他filter来的图像)
        // 画耳朵的时候  GL_ONE: 就直接使用耳朵的所有像素 原本是什么样子 我就画什么样子
        //               GL_ONE_MINUS_SRC_ALPHA: 表示用1.0减去源图颜色的alpha值来作为因子
        GLES20.glBlendFunc(GLES20.GL_ONE, GLES20.GL_ONE_MINUS_SRC_ALPHA);

        // 画耳朵 不是画全屏 定位到人脸的位置
        //拿到人脸的位置
        float x = mFace.landmarks[0];
        float y = mFace.landmarks[1];
        //转换为要画到屏幕上的宽、高
        x = x / mFace.imgWidth * mOutputWidth;
        y = y / mFace.imgHeight * mOutputHeight;

        //设置显示窗口
        //mFace.width： 人脸的宽
        GLES20.glViewport(
                (int)x,
                (int)((int)y - mStickerBitmap.getHeight()/2),
                (int)((float)mFace.width /mFace.imgWidth * mOutputWidth),
                mStickerBitmap.getHeight());

        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, mFrameBuffers[0]);
        //使用着色器
        GLES20.glUseProgram(mGLProgramId);
        //传递坐标
        mGLVertexBuffer.position(0);
        GLES20.glVertexAttribPointer(mVPosition, 2, GLES20.GL_FLOAT, false, 0, mGLVertexBuffer);
        GLES20.glEnableVertexAttribArray(mVPosition);

        mGLTextureBuffer.position(0);
        GLES20.glVertexAttribPointer(mVCoord, 2, GLES20.GL_FLOAT, false, 0, mGLTextureBuffer);
        GLES20.glEnableVertexAttribArray(mVCoord);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mBitmapTextures[0]);
        GLES20.glUniform1i(mVTexture, 0);

        //通知OpenGL 画画
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);

        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);

        //关闭
        GLES20.glDisable(GLES20.GL_BLEND);

    }

    @Override
    public void release() {
        super.release();
        if(!mStickerBitmap.isRecycled()){
            mStickerBitmap.recycle();
        }
    }
}
