package com.bryanrady.douyin.util;

import android.content.Context;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

public class GlslShaderUtils {

    /**
     * 读取着色器文件 然后拼接成字符串
     * @param context
     * @param rawId
     * @return
     */
    public static String readRawTextFile(Context context, int rawId){
        //读取顶点着色器
        InputStream is = context.getResources().openRawResource(rawId);
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
        }
        try {
            is.close();
            br.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return sb.toString();
    }

}
