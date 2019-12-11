#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include <opencv2/opencv.hpp>

using namespace cv;

ANativeWindow *nativeWindow = 0;
DetectionBasedTracker *tracker = 0;

class CascadeDetectorAdapter : public DetectionBasedTracker::IDetector{
public:
    CascadeDetectorAdapter(cv::Ptr<cv::CascadeClassifier> detector) :
            IDetector(),
            Detector(detector)
    {

        CV_Assert(detector);
    }

    //detect是DetectionBasedTracker的内部类，里面定义了一个detect纯虚函数， 所以子类必须要来实现这个函数
    void detect(const cv::Mat& Image, std::vector<cv::Rect>& objects)
    {
        Detector->detectMultiScale(Image, objects, scaleFactor, minNeighbours, 0, minObjSize, maxObjSize);
    }

    virtual ~CascadeDetectorAdapter()
    {
    }

private:
    CascadeDetectorAdapter();
    //Ptr 是opencv内部定义的一个智能指针模板类 智能指针可以自动释放
    cv::Ptr<cv::CascadeClassifier> Detector;
};


extern "C" JNIEXPORT void JNICALL
Java_com_example_facetracking_MainActivity_init(JNIEnv *env, jobject thiz, jstring model_) {
    //初始化之前判断一下
    if(tracker){
        tracker->stop();
        delete tracker;
        tracker = 0;
    }

    const char *model =env->GetStringUTFChars(model_,0);

    //创建第一个分类器和跟踪适配器
    Ptr<CascadeClassifier> classifier = makePtr<CascadeClassifier>(model);
    Ptr<CascadeDetectorAdapter> mainDetector = makePtr<CascadeDetectorAdapter>(classifier);

    //创建第二个分类器和跟踪适配器
    Ptr<CascadeClassifier> trackingClassifier = makePtr<CascadeClassifier>(model);
    Ptr<CascadeDetectorAdapter> trackingDetector = makePtr<CascadeDetectorAdapter>(trackingClassifier);

    //创建拿去用的跟踪器  这里不用智能指针，方便我们自己来控制它的生命周期
    DetectionBasedTracker::Parameters parameters;
    tracker = new DetectionBasedTracker(mainDetector, trackingDetector, parameters);

    //开启跟踪器
    tracker->run();

    env->ReleaseStringUTFChars(model_,model);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_facetracking_MainActivity_setSurface(JNIEnv *env, jobject thiz, jobject surface) {
    if(nativeWindow){
        ANativeWindow_release(nativeWindow);
        nativeWindow = 0;
        return;
    }
    nativeWindow = ANativeWindow_fromSurface(env,surface);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_facetracking_MainActivity_postData(JNIEnv *env, jobject thiz, jbyteArray data_,
                                                    jint w, jint h, jint camera_id) {
    //得到的是手机摄像头的NV21的YUV数据
    jbyte *data = env->GetByteArrayElements(data_,0);

    //Mat是个矩阵，src用来表示图像  然后调用用Mat的构造方法
    // 第一个参数 高 第二个参数 宽 第3个参数 数据类型 我们的是NV21的YUV格式数据 所以是CV_8UC1 如果是RGB数据 则是CV_8UC3
    //NV21属于YUV420P数据格式 YUV的排列方式的高 实际上是 h + h/2  h是Y的高度，UV的高是Y的高度/2 4*4矩阵的下面有4组UV数据,具体看YUV数据格式

    //将data的数据放到src中
    Mat src(h + h/2, w, CV_8UC1, data);

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

    // 然后再送去进行检测
    Mat gray;
    //转换成灰度图
    cvtColor(src, gray, COLOR_RGBA2GRAY);
    //增强对比度 (直方图均衡)
    equalizeHist(gray, gray);

    std::vector<Rect> faces;
    //定位人脸
    tracker->process(gray);
    tracker->getObjects(faces);

    for(Rect face : faces){
        //画矩形
        rectangle(src, face, Scalar(255, 0, 255));
    }

    //显示
    if(nativeWindow){
        // 因为旋转了 宽高发生了交换 所以宽、高需要交换
        //  这里使用 cols 和rows 代表 宽、高 就不用关心上面是否旋转了
    //    ANativeWindow_setBuffersGeometry(nativeWindow, w, h, WINDOW_FORMAT_RGBA_8888);    //这样会有问题
    //    ANativeWindow_setBuffersGeometry(nativeWindow, h, w, WINDOW_FORMAT_RGBA_8888);
        //这里使用 cols 和rows 代表 宽(列)、高(行) 就不用关心上面是否旋转了
        ANativeWindow_setBuffersGeometry(nativeWindow, src.cols, src.rows, WINDOW_FORMAT_RGBA_8888);
        ANativeWindow_Buffer buffer;
        //如果lock失败 return
//        if(ANativeWindow_lock(nativeWindow, &windowBuffer, 0)){
//            ANativeWindow_release(nativeWindow);
//            nativeWindow = 0;
//            return;
//        }
        //两种写法都一样
        do{
            if(ANativeWindow_lock(nativeWindow, &buffer, 0)){
                ANativeWindow_release(nativeWindow);
                nativeWindow = 0;
                break;
            }
            //src.data ：上面指定的就是RGBA数据
            //把src.data 拷贝到 buffer.bits 里去  一行一行的拷贝 *4 是因为RGBA 有4个字节 RGBA四个各占一个字节
            // buffer.stride  一行有多少个数据
            memcpy(buffer.bits, src.data, buffer.stride * buffer.height * 4);
            //解锁 提交刷新
            ANativeWindow_unlockAndPost(nativeWindow);
        }while (0);

    }

    //释放Mat release内部采用的是引用计数
    src.release();
    gray.release();

    env->ReleaseByteArrayElements(data_,data,0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_facetracking_MainActivity_release(JNIEnv *env, jobject thiz) {
    if (tracker) {
        tracker->stop();
        delete tracker;
        tracker = 0;
    }
}