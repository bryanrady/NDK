//
// Created by wangqb on 2019/12/24.
//

#include "FaceTrack.h"

FaceTrack::FaceTrack(const char *model, const char *seeta) {
    //创建第一个分类器和跟踪适配器
    Ptr<CascadeClassifier> classifier = makePtr<CascadeClassifier>(model);
    Ptr<CascadeDetectorAdapter> mainDetector = makePtr<CascadeDetectorAdapter>(classifier);

    //创建第二个分类器和跟踪适配器
    Ptr<CascadeClassifier> trackingClassifier = makePtr<CascadeClassifier>(model);
    Ptr<CascadeDetectorAdapter> trackingDetector = makePtr<CascadeDetectorAdapter>(trackingClassifier);

    //创建拿去用的跟踪器  这里不用智能指针，方便我们自己来控制它的生命周期
    DetectionBasedTracker::Parameters parameters;
    tracker = makePtr<DetectionBasedTracker>(mainDetector, trackingDetector, parameters);

    faceAlignment = makePtr<seeta::FaceAlignment>(seeta);
}

FaceTrack::~FaceTrack() {

}

void FaceTrack::detector(Mat src, vector<Rect2f> &rects) {
    vector<Rect> faces;
    //src : 灰度图
    tracker->process(src);
    tracker->getObjects(faces);
    if (faces.size()) {
        Rect face = faces[0];
        rects.push_back(Rect2f(face.x, face.y, face.width, face.height));

        //关键点定位
        //保存5个关键点的坐标
        // 0:左眼  1:右眼  2:鼻头  3:嘴巴左  4:嘴巴右
        seeta::FacialLandmark points[5];
        //图像数据
        seeta::ImageData image_data(src.cols, src.rows);
        image_data.data = src.data;
        //指定人脸部位
        seeta::FaceInfo faceInfo;
        seeta::Rect bbox;
        bbox.x = face.x;
        bbox.y  = face.y;
        bbox.width = face.width;
        bbox.height = face.height;
        faceInfo.bbox = bbox;
        faceAlignment->PointDetectLandmarks(image_data, faceInfo, points);

        for (int i = 0; i < 5; ++i) {
            //把点放入返回的集合 点的话没有宽高 就传0就行
            rects.push_back(Rect2f(points[i].x, points[i].y, 0, 0));
        }

    }
}

void FaceTrack::startTracking() {
    //开启跟踪器
    tracker->run();
}

void FaceTrack::stopTracking() {
    //关闭跟踪器
    tracker->stop();
}

