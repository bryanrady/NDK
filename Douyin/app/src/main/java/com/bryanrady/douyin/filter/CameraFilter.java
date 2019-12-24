package com.bryanrady.douyin.filter;

import android.content.Context;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;

import com.bryanrady.douyin.R;

/**
 * 采集摄像头图像数据  然后将图像数据画到FBO缓存中
 *      为什么需要画到FBO中，因为我们后续要进行多种效果处理，如我们增加滤镜效果就需要从FBO中取出数据，
 *      然后进行滤镜效果，然后再将滤镜效果后的图像输出到屏幕上
 *      所以ScreenFilter就不需要使用扩展的samplerExternalOES来采集了 只需要再CameraFilter中使用扩展的来采集即可
 */
public class CameraFilter extends AbstractFrameFilter {

    private float[] mMtx;

    public CameraFilter(Context context) {
        super(context.getApplicationContext(), R.raw.camera_vertex_shader, R.raw.camera_fragment_shader);
    }

    /**
     * 修改坐标  我们这里来修改纹理坐标
     */
    @Override
    protected void changeCoordinate() {
        super.changeCoordinate();
        mGLTextureBuffer.clear();
        //不需要显示到屏幕上  所以根据纹理坐标系得到数据  左下角  右下角  左上角  右上角
        //摄像头是颠倒的 所以我们要来进行旋转
//        float[] texture = {
//                0.0f, 0.0f,
//                1.0f, 0.0f,
//                0.0f, 1.0f,
//                1.0f, 1.0f
//        };
        //镜像  就是第一行和第2行互换  第3行和第4行互换
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
    public int onDrawFrame(int textureId) {
        //不使用父类的 自己来重写
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

    public void setMatrix(float[] mtx) {
        this.mMtx = mtx;
    }


}
