#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/bitmap.h>
#include "libgif/gif_lib.h"

#define LOG_TAG "wangqingbin"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define argb(a,r,g,b) ( ((a) & 0xff) << 24 ) | ( ((b) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((r) & 0xff)
#define dispose(ext) (((ext)->Bytes[0] & 0x1c) >> 2)
#define trans_index(ext) ((ext)->Bytes[3])
#define transparency(ext) ((ext)->Bytes[0] & 1)
#define delay(ext) (10*((ext)->Bytes[2] << 8 | (ext)->Bytes[1]))

typedef struct GifBean{
    int current_frame;  //当前播放的第几帧
    int total_frames;   //总帧数
    int *delays = 0;        //代表一个数组，存放每一帧的播放时间
} GifBean;

int drawFrame(GifFileType* gif,GifBean * gifBean, AndroidBitmapInfo  info, void* pixels,  bool force_dispose_1) {
    GifColorType *bg;

    GifColorType *color;

    SavedImage * frame;

    ExtensionBlock * ext = 0;

    GifImageDesc * frameInfo;

    ColorMapObject * colorMap;

    int *line;

    int width, height,x,y,j,loc,n,inc,p;

    void* px;



    width = gif->SWidth;

    height = gif->SHeight;



    frame = &(gif->SavedImages[gifBean->current_frame]);

    frameInfo = &(frame->ImageDesc);

    if (frameInfo->ColorMap) {

        colorMap = frameInfo->ColorMap;

    } else {

        colorMap = gif->SColorMap;

    }



    bg = &colorMap->Colors[gif->SBackGroundColor];



    for (j=0; j<frame->ExtensionBlockCount; j++) {

        if (frame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {

            ext = &(frame->ExtensionBlocks[j]);

            break;

        }

    }
    // For dispose = 1, we assume its been drawn
    px = pixels;
    if (ext && dispose(ext) == 1 && force_dispose_1 && gifBean->current_frame > 0) {
        gifBean->current_frame=gifBean->current_frame-1,
                drawFrame(gif,gifBean, info, pixels,  true);
    }

    else if (ext && dispose(ext) == 2 && bg) {

        for (y=0; y<height; y++) {

            line = (int*) px;

            for (x=0; x<width; x++) {

                line[x] = argb(255, bg->Red, bg->Green, bg->Blue);

            }

            px = (int *) ((char*)px + info.stride);

        }

    } else if (ext && dispose(ext) == 3 && gifBean->current_frame > 1) {
        gifBean->current_frame=gifBean->current_frame-2,
                drawFrame(gif,gifBean, info, pixels,  true);

    }
    px = pixels;
    if (frameInfo->Interlace) {

        n = 0;

        inc = 8;

        p = 0;

        px = (int *) ((char*)px + info.stride * frameInfo->Top);

        for (y=frameInfo->Top; y<frameInfo->Top+frameInfo->Height; y++) {

            for (x=frameInfo->Left; x<frameInfo->Left+frameInfo->Width; x++) {

                loc = (y - frameInfo->Top)*frameInfo->Width + (x - frameInfo->Left);

                if (ext && frame->RasterBits[loc] == trans_index(ext) && transparency(ext)) {

                    continue;

                }



                color = (ext && frame->RasterBits[loc] == trans_index(ext)) ? bg : &colorMap->Colors[frame->RasterBits[loc]];

                if (color)

                    line[x] = argb(255, color->Red, color->Green, color->Blue);

            }

            px = (int *) ((char*)px + info.stride * inc);

            n += inc;

            if (n >= frameInfo->Height) {

                n = 0;

                switch(p) {

                    case 0:

                        px = (int *) ((char *)pixels + info.stride * (4 + frameInfo->Top));

                        inc = 8;

                        p++;

                        break;

                    case 1:

                        px = (int *) ((char *)pixels + info.stride * (2 + frameInfo->Top));

                        inc = 4;

                        p++;

                        break;

                    case 2:

                        px = (int *) ((char *)pixels + info.stride * (1 + frameInfo->Top));

                        inc = 2;

                        p++;

                }

            }

        }

    }

    else {

        px = (int *) ((char*)px + info.stride * frameInfo->Top);

        for (y=frameInfo->Top; y<frameInfo->Top+frameInfo->Height; y++) {

            line = (int*) px;

            for (x=frameInfo->Left; x<frameInfo->Left+frameInfo->Width; x++) {

                loc = (y - frameInfo->Top)*frameInfo->Width + (x - frameInfo->Left);

                if (ext && frame->RasterBits[loc] == trans_index(ext) && transparency(ext)) {

                    continue;

                }

                color = (ext && frame->RasterBits[loc] == trans_index(ext)) ? bg : &colorMap->Colors[frame->RasterBits[loc]];

                if (color)

                    line[x] = argb(255, color->Red, color->Green, color->Blue);

            }

            px = (int *) ((char*)px + info.stride);

        }
    }

    return delay(ext);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_bryanrady_gifload_GifHandler_native_1loadGif(JNIEnv *env,jobject instance,jstring path_) {
    //gif图片路径
    const char *path = env->GetStringUTFChars(path_,0);
    //用系统函数打开一个gif文件   返回一个结构体，这个结构体为句柄
    int error;
    GifFileType *gifFileType = DGifOpenFileName(path,&error);
    //初始化GifFileType
    DGifSlurp(gifFileType);

    //开始解析gif

    GifBean *gifBean = static_cast<GifBean *>(malloc(sizeof(GifBean)));
    memset(gifBean, 0, sizeof(GifBean));

    //绑定用户数据
    gifFileType->UserData = gifBean;

    gifBean->current_frame = 0;
    gifBean->total_frames = gifFileType->ImageCount;
    gifBean->delays = static_cast<int *>(malloc(sizeof(int) * gifBean->total_frames));
    memset(gifBean->delays, 0, sizeof(int) * gifBean->total_frames);

    //遍历每一帧 找到每一帧的图形控制扩展块 然后找到每一帧的延迟时间
    for (int i = 0; i < gifBean->total_frames; ++i) {
         //得到一帧图片
        SavedImage frame = gifFileType->SavedImages[i];
        ExtensionBlock *extensionBlock = 0;
        //只有89a版本的才有图形控制扩展块
        //遍历每一个扩展块  一共有4个扩展块  图形文本扩展块 图形控制扩展块 注释扩展块 应用程序扩展块
        for (int j = 0; j < frame.ExtensionBlockCount; ++j) {
            //Function 扩展块的标识  0xf9 代表图形控制扩展块
            if(frame.ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE){
                extensionBlock = &frame.ExtensionBlocks[j];
                break;
            }
        }
        if(extensionBlock){
            //延迟时间  两个字节表示一个int    Bytes[2]高八位  Bytes[1]低八位
            //delay 单位 1/100 s  也就是10ms
            int delay = (extensionBlock->Bytes[2] << 8 | extensionBlock->Bytes[1]) * 10;
            LOGE("第 d% 帧的延迟时间: %d", i, delay);
            gifBean->delays[i] = delay;
        }
    }

    env->ReleaseStringUTFChars(path_,path);
    return reinterpret_cast<jlong>(gifFileType);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_bryanrady_gifload_GifHandler_native_1getWidth(JNIEnv *env, jobject instance, jlong ndk_gif) {
    GifFileType *gifFileType = reinterpret_cast<GifFileType *>(ndk_gif);
    return gifFileType->SWidth;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_bryanrady_gifload_GifHandler_native_1getHeight(JNIEnv *env, jobject instance, jlong ndk_gif) {
    GifFileType *gifFileType = reinterpret_cast<GifFileType *>(ndk_gif);
    //gif图片的高
    return gifFileType->SHeight;
}

/**
 * 绘制bitmap
 * @param gifFileType
 * @param gifBean
 * @param info
 * @param pixels
 */
void drawBitmap(GifFileType *gifFileType, GifBean *gifBean, AndroidBitmapInfo info, void *pixels)  {
    //当前帧
    SavedImage savedImage = gifFileType->SavedImages[gifBean->current_frame];

    //每一帧的描述信息 里面有偏移信息
    GifImageDesc frameDesc = savedImage.ImageDesc;

    //整个Bitmap的首地址
    int *px = static_cast<int *>(pixels);

    //每一行的首地址
    int *line = 0;

    //用来解压缩的字典
    ColorMapObject *colorMapObject = frameDesc.ColorMap;
    px = (int *) ((char*)px + info.stride * frameDesc.Top);

    GifByteType gifByteType;
    GifColorType gifColorType;

    //先遍历行  内容的区域
    for (int row = frameDesc.Top; row < frameDesc.Top + frameDesc.Height; ++row) {
        line = px;
        //然后遍历列
        for (int column = frameDesc.Left; column < frameDesc.Left + frameDesc.Width ; ++column) {
            // 索引
            int pointPixel = (row - frameDesc.Top) * frameDesc.Width + (column - frameDesc.Left);
            //当前帧的像素数据   压缩数据  lzw算法
            gifByteType = savedImage.RasterBits[pointPixel];
            //字典
            gifColorType= colorMapObject->Colors[gifByteType];
            line[column] = argb(255, gifColorType.Red, gifColorType.Green, gifColorType.Blue);
        }

        px = (int *) ((char*)px + info.stride);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_bryanrady_gifload_GifHandler_native_1updateFrame(JNIEnv *env, jobject instance, jlong ndk_gif,
                                                  jobject bitmap) {
    GifFileType *gifFileType = reinterpret_cast<GifFileType *>(ndk_gif);
    GifBean *gifBean = static_cast<GifBean *>(gifFileType->UserData);
    //获取bitmap的信息
    //uint32_t stride;  bitmap每一行的像素
    AndroidBitmapInfo info;
    AndroidBitmap_getInfo(env,bitmap,&info);

    //bitmap 转换缓冲区   byte[]
    void *pixels = 0;
    AndroidBitmap_lockPixels(env,bitmap,&pixels);

    //绘制bitmap  只支持89a的版本
    //drawBitmap(gifFileType,gifBean,info,pixels);

    //项目中用的  既支持87a版本 又支持89a版本
    drawFrame(gifFileType, gifBean, info, pixels, false);

    //绘制完成后 对当前帧进行+1
    gifBean->current_frame += 1;
    LOGE("当前帧  %d  ",gifBean->current_frame);
    //播放完了 继续播放
    if (gifBean->current_frame >= gifBean->total_frames - 1) {
        gifBean->current_frame = 0;
        LOGE("重新播放  %d  ",gifBean->current_frame);
    }
    //解锁
    AndroidBitmap_unlockPixels(env,bitmap);
    //当前帧播放完成后 就返回下一帧的延迟时间
    return gifBean->delays[gifBean->current_frame];
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_gifload_GifHandler_native_1release(JNIEnv *env, jobject instance, jlong ndk_gif) {
    GifFileType *gifFileType = reinterpret_cast<GifFileType *>(ndk_gif);
    GifBean *gifBean = static_cast<GifBean *>(gifFileType->UserData);

    free(gifBean->delays);
    gifBean->delays = 0;
    free(gifBean);
    gifBean = 0;

    DGifCloseFile(gifFileType);
    gifFileType = 0;
}