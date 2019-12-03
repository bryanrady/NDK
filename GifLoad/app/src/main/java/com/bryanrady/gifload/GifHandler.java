package com.bryanrady.gifload;

import android.graphics.Bitmap;

/**
 * @author: wangqingbin
 * @date: 2019/12/3 10:05
 */
public class GifHandler {

    //这个gif的地址是一个结构体的地址，用来和native打交道
    private long mGifAddr;

    static {
        System.loadLibrary("native-lib");
    }

    public GifHandler(String path){
        mGifAddr = native_loadGif(path);
    }

    public int getWidth(){
        return native_getWidth(mGifAddr);
    }

    public int getHeight(){
        return native_getHeight(mGifAddr);
    }

    public int updateFrame(Bitmap bitmap){
        return native_updateFrame(mGifAddr,bitmap);
    }

    public void release(){
        native_release(mGifAddr);
    }

    private native long native_loadGif(String path);

    private native int native_getWidth(long ndkGif);

    private native int native_getHeight(long ndkGif);

    private native int native_updateFrame(long ndkGif,Bitmap bitmap);

    private native void native_release(long ndkGif);
}
