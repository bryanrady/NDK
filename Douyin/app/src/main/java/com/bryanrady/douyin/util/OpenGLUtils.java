package com.bryanrady.douyin.util;

import android.content.Context;
import android.opengl.GLES20;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

public class OpenGLUtils {

    /**
     * 读取着色器文件 然后拼接成字符串
     * @param context
     * @param rawId
     * @return
     */
    public static String readRawTextFile(Context context, int rawId){
        //读取顶点着色器
        InputStream is = context.getApplicationContext().getResources().openRawResource(rawId);
        BufferedReader br = new BufferedReader(new InputStreamReader(is));
        String line;
        StringBuffer sb = new StringBuffer();
        try {
            while ((line = br.readLine()) != null){
                sb.append(line);
                sb.append("\n");
            }
        }catch (Exception e){
            e.printStackTrace();
        }finally {
            try {
                is.close();
                br.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return sb.toString();
    }

    /**
     * 加载着色器并创建着色器小程序
     * 通过字符串(代码) vertexSource fragmentSource 创建着色器小程序
     * @param vertexSource
     * @param fragmentSource
     * @return
     */
    public static int loadProgram(String vertexSource,String fragmentSource){
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
            //throw new IllegalStateException("ScreenFilter 顶点着色器配置失败!");
            throw new IllegalStateException("load vertex shader:"+GLES20.glGetShaderInfoLog(vertexShaderId));
        }

        //2.配置片元着色器
        int fragmentShaderId = GLES20.glCreateShader(GLES20.GL_FRAGMENT_SHADER);
        GLES20.glShaderSource(fragmentShaderId, fragmentSource);
        GLES20.glCompileShader(fragmentShaderId);
        GLES20.glGetShaderiv(fragmentShaderId, GLES20.GL_COMPILE_STATUS, status, 0);
        if(status[0] != GLES20.GL_TRUE){
            //throw new IllegalStateException("ScreenFilter 片元着色器配置失败!");
            throw new IllegalStateException("load fragment shader:"+GLES20.glGetShaderInfoLog(fragmentShaderId));
        }

        //3.创建着色器小程序(GPU上面的小程序)
        int program = GLES20.glCreateProgram();
        //把着色器附加到程序当中
        GLES20.glAttachShader(program, vertexShaderId);
        GLES20.glAttachShader(program, fragmentShaderId);
        //链接着色器
        GLES20.glLinkProgram(program);

        GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS, status, 0);
        if (status[0] != GLES20.GL_TRUE) {
            //throw new IllegalStateException("ScreenFilter 着色器程序配置失败!");
            throw new IllegalStateException("load vertex shader:"+GLES20.glGetProgramInfoLog(program));
        }

        //4. 删除着色器 因为已经附加到着色器程序中了，所以释放掉了没关系
        GLES20.glDeleteShader(vertexShaderId);
        GLES20.glDeleteShader(fragmentShaderId);

        return program;
    }

    /**
     * 创建纹理并配置
     * @param textures
     */
    public static void glGenTextures(int[] textures){
        //创建纹理
        GLES20.glGenTextures(textures.length, textures, 0);
        for (int i = 0; i < textures.length; i++){
            //OpenGL的操作是面向过程的操作
            //bind就是绑定 ，表示后续的操作就是在这一个 纹理上进行，后面的代码配置纹理，就是配置bind的这个纹理
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textures[i]);
            /**
             * 配置过滤参数
             *  当纹理被使用到一个比他大 或者比他小的形状上的时候 该如何处理(缩放 就是放大或者缩小)
             */
            // 放大
            // 第3个参数：GLES20.GL_LINEAR  : 使用纹理中坐标附近的若干个颜色，通过平均算法 进行放大
            //            GLES20.GL_NEAREST : 使用纹理坐标最接近的一个颜色作为放大的要绘制的颜色
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_NEAREST);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);

            /**
             * 配置纹理环绕方向
             */
            //纹理坐标 一般用st表示，其实就是x y
            //纹理坐标的范围是0-1。超出这一范围的坐标将被OpenGL根据GL_TEXTURE_WRAP参数的值进行处理
            //GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T 分别为x，y方向。
            //GL_REPEAT:平铺
            //GL_MIRRORED_REPEAT: 纹理坐标是奇数时使用镜像平铺
            //GL_CLAMP_TO_EDGE: 坐标超出部分被截取成0、1，边缘拉伸
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);

            //解绑 将纹理置为0
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        }
    }

    public static void copyAssets2SdCard(Context context, String src, String dst) {
        try {
            File file = new File(dst);
            if (!file.exists()) {
                InputStream is = context.getApplicationContext().getAssets().open(src);
                FileOutputStream fos = new FileOutputStream(file);
                int len;
                byte[] buffer = new byte[2048];
                while ((len = is.read(buffer)) != -1) {
                    fos.write(buffer, 0, len);
                }
                is.close();
                fos.close();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}
