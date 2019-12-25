package com.bryanrady.douyin.filter;

import android.content.Context;
import android.opengl.GLES20;

import com.bryanrady.douyin.R;
import com.bryanrady.douyin.face.Face;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

/**
 * 大眼效果 对一张人脸找到了关键点的各个部位后，对眼睛进行局部缩放就达到了大眼效果，而且不影响其他关键部位
 */
public class BigEyeFilter extends AbstractFrameFilter {

    private int mLeft_Eye;
    private int mRight_Eye;

    private FloatBuffer mLeftBuffer;
    private FloatBuffer mRightBuffer;

    private Face mFace;

    public BigEyeFilter(Context context) {
        super(context.getApplicationContext(), R.raw.base_vertex_shader, R.raw.bigeye_fragment_shader);
        mLeft_Eye = GLES20.glGetUniformLocation(mGLProgramId, "left_eye");
        mRight_Eye = GLES20.glGetUniformLocation(mGLProgramId, "right_eye");

        mLeftBuffer = ByteBuffer.allocateDirect(2 * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        mRightBuffer = ByteBuffer.allocateDirect(2 * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
    }

    public void setFace(Face face) {
        mFace = face;
    }

    @Override
    public int onDrawFrame(int textureId) {
        if (null == mFace) {
            return textureId;
        }
        //设置显示窗口
        GLES20.glViewport(0, 0, mOutputWidth, mOutputHeight);

        //不调用的话就是默认的操作GLSurfaceView中的纹理了,显示到屏幕上了
        //这里我们还只是把它画到fbo中(缓存)
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

        /**
         * 传递眼睛的坐标 给GLSL
         */
        float[] landmarks = mFace.landmarks;
        //下标为0、1时 : 保存人脸的 x与y  所以眼睛的x y 从2 3开始
        //左眼的x 、y  OpenGL接收的坐标是 : 0 到 1 ，所以这里除以人脸的x y 映射成 0 到 1 的数据
        float x = landmarks[2] / mFace.imgWidth;
        float y = landmarks[3] / mFace.imgHeight;
        mLeftBuffer.clear();
        mLeftBuffer.put(x);
        mLeftBuffer.put(y);
        mLeftBuffer.position(0);
        GLES20.glUniform2fv(mLeft_Eye,1, mLeftBuffer);

        //右眼的x、y
        x = landmarks[4] / mFace.imgWidth;
        y = landmarks[5] / mFace.imgHeight;
        mRightBuffer.clear();
        mRightBuffer.put(x);
        mRightBuffer.put(y);
        mRightBuffer.position(0);
        GLES20.glUniform2fv(mRight_Eye,1, mRightBuffer);


        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        //这里不再需要使用扩展的了
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId);
        GLES20.glUniform1i(mVTexture, 0);

        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);

        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);

        //返回fbo的纹理id
        return mFrameBufferTextures[0];
    }
}
