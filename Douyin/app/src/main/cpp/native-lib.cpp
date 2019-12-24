#include <jni.h>
#include <string>
#include <opencv2/core.hpp>

#include "FaceTrack.h"

extern "C"
JNIEXPORT jlong JNICALL
Java_com_bryanrady_douyin_face_FaceTrack_native_1create(JNIEnv *env, jobject instance, jstring model_,
                                                        jstring seeta_) {
    const char *model = env->GetStringUTFChars(model_, 0);
    const char *seeta = env->GetStringUTFChars(seeta_, 0);

    FaceTrack *faceTrack = new FaceTrack(model, seeta);

    env->ReleaseStringUTFChars(model_, model);
    env->ReleaseStringUTFChars(seeta_, seeta);

    return reinterpret_cast<jlong>(faceTrack);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_douyin_face_FaceTrack_native_1start(JNIEnv *env, jobject instance, jlong self) {
    if(self == 0){
        return;
    }
    FaceTrack *faceTrack = reinterpret_cast<FaceTrack *>(self);
    faceTrack->startTracking();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_douyin_face_FaceTrack_native_1stop(JNIEnv *env, jobject instance, jlong self) {
    if(self == 0){
        return;
    }
    FaceTrack *faceTrack = reinterpret_cast<FaceTrack *>(self);
    faceTrack->stopTracking();

    delete faceTrack;
    faceTrack = 0;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_bryanrady_douyin_face_FaceTrack_native_1detector(JNIEnv *env, jobject instance, jlong self,
                                                          jbyteArray data_, jint camera_id,
                                                          jint width, jint height) {
    if(self == 0){
        return NULL;
    }

    FaceTrack *faceTrack = reinterpret_cast<FaceTrack *>(self);

    //得到的是手机摄像头的NV21的YUV数据
    jbyte *data = env->GetByteArrayElements(data_,0);

    //Mat是个矩阵，src用来表示图像  然后调用用Mat的构造方法
    // 第一个参数 高 第二个参数 宽 第3个参数 数据类型 我们的是NV21的YUV格式数据 所以是CV_8UC1 如果是RGB数据 则是CV_8UC3
    //NV21属于YUV420P数据格式 YUV的排列方式的高 实际上是 h + h/2  h是Y的高度，UV的高是Y的高度/2 4*4矩阵的下面有4组UV数据,具体看YUV数据格式

    //将data的数据放到src中
    Mat src(height + height/2, width, CV_8UC1, data);

    //将NV21的YUV数据转成RGBA 格式
    cvtColor(src, src, COLOR_YUV2RGBA_NV21);

    //正在写的过程 如果我们退出了，会导致文件丢失数据
    //现在的src图像是颠倒的，因为我们在java层没有进行旋转，我们通过opencv来进行旋转
    //将图像写到sd卡中的src.jpg   如果通过RGBA数据写上去，我们看到的图像是个绿色的图像，所以我们要把上面改成BGR格式，早期的计算机处理的是BGR数据
    //imwrite("/sdcard/src.jpg",src);

    //在这里旋转图像比java简单多了
    if(camera_id == 1){
        //前置摄像头需要逆时针旋转90度
        rotate(src, src, ROTATE_90_COUNTERCLOCKWISE);
        //水平翻转 镜像 如果我们要达到和手机自带相机一样的效果的话需要镜像翻转得到镜像
        //1 代表水平翻转 0 代表垂直翻转
        flip(src, src, 1);
    }else{
        //后置摄像头需要顺时针旋转90度
        rotate(src, src, ROTATE_90_CLOCKWISE);
    }

    //转换成灰度图
    cvtColor(src, src, COLOR_RGBA2GRAY);
    //增强对比度 (直方图均衡)
    equalizeHist(src, src);

    vector<Rect2f> rects;
    //定位人脸
    //送去定位
    faceTrack->detector(src, rects);
    env->ReleaseByteArrayElements(data_, data, 0);

    int w = src.cols;
    int h = src.rows;
    src.release();
    int size = rects.size();
    if (size) {
        jclass clazz = env->FindClass("com/bryanrady/douyin/face/Face");
        jmethodID costruct = env->GetMethodID(clazz, "<init>", "(IIII[F)V");
        int floatSize = size * 2;
        //创建java 的float 数组
        jfloatArray floatArray = env->NewFloatArray(floatSize);
        for (int i = 0, j = 0; i < floatSize; j++) {
            float f[2] = {rects[j].x, rects[j].y};
            env->SetFloatArrayRegion(floatArray, i, 2, f);
            i += 2;
        }
        Rect2f faceRect = rects[0];
        int width = faceRect.width;
        int height = faceRect.height;
        jobject face = env->NewObject(clazz, costruct, width, height, w, h, floatArray);
        return face;
    }
    return NULL;
}