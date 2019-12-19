package com.bryanrady.douyin.filter;

import android.content.Context;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;

import com.bryanrady.douyin.R;
import com.bryanrady.douyin.util.GlslShaderUtils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

/**
 * 负责将摄像头的数据渲染到屏幕上
 */
public class ScreenFilter {

    private int mProgram;
    private int mWidth;
    private int mHeight;
    private int mVPosition;
    private int mVTexture;
    private int mVMatrix;
    private int mVCoord;
    private FloatBuffer mVertexBuffer;
    private FloatBuffer mTextureBuffer;

    public ScreenFilter(Context context){
        //读取着色器文件
        String vertexSource = GlslShaderUtils.readRawTextFile(context, R.raw.camera_vertex_shader);
        String fragmentSource = GlslShaderUtils.readRawTextFile(context, R.raw.camera_fragment_shader);
        /**
         * 着色器的使用
         * 通过字符串(代码) vertexSource fragmentSource 创建着色器小程序
         */
        //1.配置顶点着色器
        //1.1 创建顶点着色器
        int vertexShaderId = GLES20.glCreateShader(GLES20.GL_VERTEX_SHADER);
        //1.2 绑定代码到着色器中去
        GLES20.glShaderSource(vertexShaderId, vertexSource);
        //1.3 编译着色器
        GLES20.glCompileShader(vertexShaderId);
        //主动获取成功、失败 (如果不主动查询，只输出 一条 GLERROR之类的日志，很难定位到到底是那里出错)
        int[] status = new int[1];
        GLES20.glGetShaderiv(vertexShaderId, GLES20.GL_COMPILE_STATUS, status, 0);
        if (status[0] != GLES20.GL_TRUE){
            throw new IllegalStateException("ScreenFilter 顶点着色器配置失败!");
        }

        //2.配置片元着色器
        int fragmentShaderId = GLES20.glCreateShader(GLES20.GL_FRAGMENT_SHADER);
        GLES20.glShaderSource(fragmentShaderId, fragmentSource);
        GLES20.glCompileShader(fragmentShaderId);
        GLES20.glGetShaderiv(fragmentShaderId, GLES20.GL_COMPILE_STATUS, status, 0);
        if(status[0] != GLES20.GL_TRUE){
            throw new IllegalStateException("ScreenFilter 片元着色器配置失败!");
        }

        //3.创建着色器小程序(GPU上面的小程序)
        mProgram = GLES20.glCreateProgram();
        //把着色器附加到程序当中
        GLES20.glAttachShader(mProgram, vertexShaderId);
        GLES20.glAttachShader(mProgram, fragmentShaderId);
        //链接着色器
        GLES20.glLinkProgram(mProgram);

        GLES20.glGetProgramiv(mProgram, GLES20.GL_COMPILE_STATUS, status, 0);
        if (status[0] != GLES20.GL_TRUE) {
            throw new IllegalStateException("ScreenFilter 着色器程序配置失败!");
        }

        //4. 删除着色器 因为已经附加到着色器程序中了，所以释放掉了没关系
        GLES20.glDeleteShader(vertexShaderId);
        GLES20.glDeleteShader(fragmentShaderId);


        /**
         * 获得着色器程序中的变量的索引， 通过这个索引来给着色器中的变量赋值
         */
        //获得顶点着色器的3个变量索引
        mVPosition = GLES20.glGetAttribLocation(mProgram,"vPosition");
        mVCoord = GLES20.glGetAttribLocation(mProgram, "vCoord");
        mVMatrix = GLES20.glGetUniformLocation(mProgram, "vMatrix");
        //获得片元着色器的vTexture变量的索引
        mVTexture = GLES20.glGetUniformLocation(mProgram, "vTexture");

        //Java Nio的Buffer
        //创建一个顶点坐标数据缓冲区 用来记录顶点坐标
        //4个顶点 每个顶点2个数据(x,y) 数据类型float(4个字节) 所以 4 * 2 * 4
        mVertexBuffer = ByteBuffer.allocateDirect(4 * 2 * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        mVertexBuffer.clear();
        //世界坐标系 中心点为原点(0, 0)   左下角  右下角  左上角  右上角
        //点1 点2 点3 构成一个三角形 点2 点3 点4 构成一个三角形 这样两个三角形就能构成一个完整的矩形
        float[] vertex = {
                -1.0f, -1.0f,
                1.0f,  -1.0f,
                -1.0f, 1.0f,
                1.0f,  1.0f
        };
        mVertexBuffer.put(vertex);

        //创建一个采样纹理坐标数据缓冲区 用来记录纹理坐标
        mTextureBuffer = ByteBuffer.allocateDirect(4 * 2 * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        mTextureBuffer.clear();
//        float[] texture = {0.0f, 1.0f,
//                1.0f, 1.0f,
//                0.0f, 0.0f,
//                1.0f, 0.0f};
        //旋转
//        float[] texture = {1.0f, 1.0f,
//                1.0f, 0.0f,
//                0.0f, 1.0f,
//                0.0f, 0.0f};
        //镜像
        float[] texture = {1.0f, 0.0f,
                1.0f, 1.0f,
                0.0f, 0.0f,
                0.0f, 1.0f
        };
        mTextureBuffer.put(texture);

    }

    /**
     * 使用着色器程序进行绘制
     * @param texture   纹理
     * @param mtx
     */
    public void onDrawFrame(int texture,float[] mtx){
        //1、设置窗口画布的大小
        //画画的时候 画布可以看成 10x10，也可以看成5x5 等等
        //设置画布的大小，然后画画的时候， 画布越大，你画上去的图像就会显得越小
        // x与y 就是从画布的哪个位置开始画
        GLES20.glViewport(0, 0, mWidth, mHeight);

        //2.使用着色器程序
        GLES20.glUseProgram(mProgram);

        //3.绘制顶点  怎么绘制？ 其实就是传值，然后OpenGL就会自动为我们去绘制
        //3.1  将顶点坐标数据传入，确定形状
        //(1) 顶点坐标数据
        //(2) xy两个数据
        //(3) float类型
        GLES20.glVertexAttribPointer(mVPosition,2, GLES20.GL_FLOAT, false,0, mVertexBuffer);
        //传了数据之后 激活
        GLES20.glEnableVertexAttribArray(mVPosition);

        //3.2 将纹理坐标传入，采样坐标
        mTextureBuffer.position(0);
        GLES20.glVertexAttribPointer(mVCoord, 2, GLES20.GL_FLOAT, false, 0, mTextureBuffer);
        GLES20.glEnableVertexAttribArray(mVCoord);

        //3.3 变换矩阵
        GLES20.glUniformMatrix4fv(mVMatrix,1,false, mtx,0);

        //3.4 绘制片元
        //激活图层 就是激活多层纹理
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        // 绑定图像数据到采样器
        // 正常：GLES20.GL_TEXTURE_2D
        // SurfaceTexture的纹理需要  GL_TEXTURE_EXTERNAL_OES
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, texture);
        //传递参数 0：需要和纹理层GL_TEXTURE0对应
        GLES20.glUniform1i(mVTexture,0);

        //4. 参数传完了 通知OpenGL 画画 从第0点开始 共4个点
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP,0,4);

    }

    public void onReady(int width, int height){
        mWidth = width;
        mHeight = height;
    }

}
