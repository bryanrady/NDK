//
// Created by wangqb on 2019/12/24.
//

#ifndef DOUYIN_FACETRACK_H
#define DOUYIN_FACETRACK_H

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <vector>
#include <face_alignment.h>

using namespace std;
using namespace cv;

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

class FaceTrack{
public:
    FaceTrack(const char *model, const char *seeta);
    ~FaceTrack();

    void detector(Mat src, vector<Rect2f> &rects);

    void startTracking();

    void stopTracking();

private:
    //人脸定位
    Ptr<DetectionBasedTracker> tracker;
    //人脸关键点定位
    Ptr<seeta::FaceAlignment> faceAlignment;
};

#endif //DOUYIN_FACETRACK_H
