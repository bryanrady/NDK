package com.bryanrady.douyin.filter;

import android.content.Context;
import android.opengl.GLES20;

import com.bryanrady.douyin.util.OpenGLUtils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;


/**
 *  世界坐标系
 *      (-1,1)     (0,1)     (1,1)
 *
 *      (-1,0)     (0,0)     (1,0)
 *
 *      (-1,-1)    (0,-1)    (1,-1)
 *
 *   纹理坐标系
 *
 *      (0,1)                (1,1)
 *
 *
 *
 *      (0,0)                (1,0)
 *
 *
 *   android屏幕坐标系
 *
 *      (0,0)                (1,0)
 *
 *
 *
 *      (0,1)                (1,1)
 */
public abstract class AbstractFilter {

    protected int mVertexShaderId;
    protected int mFragmentShaderId;
    protected int mGLProgramId;

    protected int mVPosition;
    protected int mVTexture;
    protected int mVMatrix;
    protected int mVCoord;

    protected FloatBuffer mGLVertexBuffer;
    protected FloatBuffer mGLTextureBuffer;

    protected int mOutputWidth;
    protected int mOutputHeight;

    public AbstractFilter(Context context, int vertexShaderId, int fragmentShaderId) {
        mVertexShaderId = vertexShaderId;
        mFragmentShaderId = fragmentShaderId;

        //Java Nio的Buffer
        //创建一个顶点坐标数据缓冲区 用来记录顶点坐标
        //4个顶点 每个顶点2个数据(x,y) 数据类型float(4个字节) 所以 4 * 2 * 4
        mGLVertexBuffer = ByteBuffer.allocateDirect(4 * 2 * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        mGLVertexBuffer.clear();
        //世界坐标系 中心点为原点(0, 0)   左下角  右下角  左上角  右上角
        //点1 点2 点3 构成一个三角形 点2 点3 点4 构成一个三角形 这样两个三角形就能构成一个完整的矩形
        float[] vertex = {
                -1.0f, -1.0f,
                1.0f,  -1.0f,
                -1.0f, 1.0f,
                1.0f,  1.0f
        };
        mGLVertexBuffer.put(vertex);

        //创建一个采样纹理坐标数据缓冲区 用来记录纹理坐标
        mGLTextureBuffer = ByteBuffer.allocateDirect(4 * 2 * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        mGLTextureBuffer.clear();

        //需要显示到屏幕上 所以根据Android屏幕坐标系得到数据 左下角  右下角  左上角  右上角
        float[] texture = {
                0.0f, 1.0f,
                1.0f, 1.0f,
                0.0f, 0.0f,
                1.0f, 0.0f
        };
        //经过旋转
//        float[] texture = {1.0f, 1.0f,
//                1.0f, 0.0f,
//                0.0f, 1.0f,
//                0.0f, 0.0f};
        //然后镜像  这样才得到 正的图像
//        float[] texture = {1.0f, 0.0f,
//                1.0f, 1.0f,
//                0.0f, 0.0f,
//                0.0f, 1.0f
//        };
        mGLTextureBuffer.put(texture);

        //初始化着色器程序
        initialize(context);
        //空实现 用来修改坐标
        changeCoordinate();

    }

    /**
     * 初始化
     * @param context
     */
    protected void initialize(Context context) {
        //读取着色器文件
        String vertexSource = OpenGLUtils.readRawTextFile(context, mVertexShaderId);
        String fragmentSource = OpenGLUtils.readRawTextFile(context, mFragmentShaderId);
        //加载着色器并创建着色器小程序
        mGLProgramId = OpenGLUtils.loadProgram(vertexSource, fragmentSource);
        /**
         * 获得着色器程序中的变量的索引， 通过这个索引来给着色器中的变量赋值
         */
        //获得顶点着色器的3个变量索引
        mVPosition = GLES20.glGetAttribLocation(mGLProgramId,"vPosition");
        mVCoord = GLES20.glGetAttribLocation(mGLProgramId, "vCoord");
        mVMatrix = GLES20.glGetUniformLocation(mGLProgramId, "vMatrix");
        //获得片元着色器的vTexture变量的索引
        mVTexture = GLES20.glGetUniformLocation(mGLProgramId, "vTexture");
    }

    /**
     * 使用着色器程序进行绘制
     * @param textureId   纹理id
     * @param mtx
     */
    public void onDrawFrame2(int textureId,float[] mtx){
        //1、设置窗口画布的大小
        //画画的时候 画布可以看成 10x10，也可以看成5x5 等等
        //设置画布的大小，然后画画的时候， 画布越大，你画上去的图像就会显得越小
        // x与y 就是从画布的哪个位置开始画
        GLES20.glViewport(0, 0, mOutputWidth, mOutputHeight);

        //2.使用着色器程序
        GLES20.glUseProgram(mGLProgramId);

        //3.绘制顶点  怎么绘制？ 其实就是传值，然后OpenGL就会自动为我们去绘制
        //3.1  将顶点坐标数据传入，确定形状
        //(1) 顶点坐标数据
        //(2) xy两个数据
        //(3) float类型
        mGLVertexBuffer.position(0);
        GLES20.glVertexAttribPointer(mVPosition,2, GLES20.GL_FLOAT, false,0, mGLVertexBuffer);
        //传了数据之后 激活
        GLES20.glEnableVertexAttribArray(mVPosition);

        //3.2 将纹理坐标传入，采样坐标
        mGLTextureBuffer.position(0);
        GLES20.glVertexAttribPointer(mVCoord, 2, GLES20.GL_FLOAT, false, 0, mGLTextureBuffer);
        GLES20.glEnableVertexAttribArray(mVCoord);

        //3.3 变换矩阵
        GLES20.glUniformMatrix4fv(mVMatrix,1,false, mtx,0);

        //3.4 绘制片元
        //激活图层 就是激活多层纹理
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        // 绑定图像数据到采样器
        // 正常：GLES20.GL_TEXTURE_2D
        // SurfaceTexture的纹理需要  GL_TEXTURE_EXTERNAL_OES
        //GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId);
        //传递参数 0：需要和纹理层GL_TEXTURE0对应
        GLES20.glUniform1i(mVTexture,0);

        //4. 参数传完了 通知OpenGL 画画 从第0点开始 共4个点
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP,0,4);
    }

    /**
     * 使用着色器程序进行绘制
     * @param textureId 纹理id
     * @return
     */
    public int onDrawFrame(int textureId){
        //1、设置窗口画布的大小
        //画画的时候 画布可以看成 10x10，也可以看成5x5 等等
        //设置画布的大小，然后画画的时候， 画布越大，你画上去的图像就会显得越小
        // x与y 就是从画布的哪个位置开始画
        GLES20.glViewport(0, 0, mOutputWidth, mOutputHeight);

        //2.使用着色器程序
        GLES20.glUseProgram(mGLProgramId);

        //3.绘制顶点  怎么绘制？ 其实就是传值，然后OpenGL就会自动为我们去绘制
        //3.1  将顶点坐标数据传入，确定形状
        //(1) 顶点坐标数据
        //(2) xy两个数据
        //(3) float类型
        mGLVertexBuffer.position(0);
        GLES20.glVertexAttribPointer(mVPosition,2, GLES20.GL_FLOAT, false,0, mGLVertexBuffer);
        //传了数据之后 激活
        GLES20.glEnableVertexAttribArray(mVPosition);

        //3.2 将纹理坐标传入，采样坐标
        mGLTextureBuffer.position(0);
        GLES20.glVertexAttribPointer(mVCoord, 2, GLES20.GL_FLOAT, false, 0, mGLTextureBuffer);
        GLES20.glEnableVertexAttribArray(mVCoord);

        //3.3 绘制片元
        //激活图层 就是激活多层纹理
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        // 绑定图像数据到采样器
        // 正常：GLES20.GL_TEXTURE_2D
        // SurfaceTexture的纹理需要  GL_TEXTURE_EXTERNAL_OES
        //GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, texture);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId);
        //传递参数 0：需要和纹理层GL_TEXTURE0对应
        GLES20.glUniform1i(mVTexture,0);

        //4. 参数传完了 通知OpenGL 画画 从第0点开始 共4个点
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP,0,4);

        return textureId;
    }

    public void onReady(int width, int height) {
        mOutputWidth = width;
        mOutputHeight = height;
    }

    public void release() {
        GLES20.glDeleteProgram(mGLProgramId);
    }

    /**
     * 修改坐标
     */
    protected void changeCoordinate() {

    }
}
