package com.bryanrady.douyin.filter;

import android.content.Context;
import android.opengl.GLES20;

import com.bryanrady.douyin.R;

/**
 * 美颜
 */
public class BeautyFilter extends AbstractFrameFilter {

    private int mWidth;
    private int mHeight;

    public BeautyFilter(Context context) {
        super(context.getApplicationContext(), R.raw.base_vertex_shader, R.raw.beauty_fragment_shader);

        mWidth = GLES20.glGetUniformLocation(mGLProgramId, "width");
        mHeight = GLES20.glGetUniformLocation(mGLProgramId, "height");
    }

    @Override
    public int onDrawFrame(int textureId) {
        //设置显示窗口
        GLES20.glViewport(0, 0, mOutputWidth, mOutputHeight);

        //不调用的话就是默认的操作GLSurfaceView中的纹理了,显示到屏幕上了
        //这里我们还只是把它画到fbo中(缓存)
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, mFrameBuffers[0]);

        //使用着色器
        GLES20.glUseProgram(mGLProgramId);

        GLES20.glUniform1i(mWidth, mOutputWidth);
        GLES20.glUniform1i(mHeight, mOutputHeight);

        //传递坐标
        mGLVertexBuffer.position(0);
        GLES20.glVertexAttribPointer(mVPosition, 2, GLES20.GL_FLOAT, false, 0, mGLVertexBuffer);
        GLES20.glEnableVertexAttribArray(mVPosition);

        mGLTextureBuffer.position(0);
        GLES20.glVertexAttribPointer(mVCoord, 2, GLES20.GL_FLOAT, false, 0, mGLTextureBuffer);
        GLES20.glEnableVertexAttribArray(mVCoord);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId);
        GLES20.glUniform1i(mVTexture, 0);

        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);

        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);

        return mFrameBufferTextures[0];
    }
}
