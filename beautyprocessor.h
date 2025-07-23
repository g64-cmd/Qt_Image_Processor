// beautyprocessor.h
#ifndef BEAUTYPROCESSOR_H
#define BEAUTYPROCESSOR_H

#include <QImage>
#include <QTemporaryFile>
#include <memory>
#include <opencv2/opencv.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/image_io.h>

class BeautyProcessor
{
public:
    BeautyProcessor();

    QImage process(const QImage &sourceImage, int smoothLevel, int thinLevel);

private:
    void applySkinSmoothing(cv::Mat &image, const dlib::full_object_detection& landmarks, int level);
    void applyFaceThinning(cv::Mat &image, const dlib::full_object_detection& landmarks, int level);

    // --- UPDATED: Replaced CascadeClassifier with dlib's detector ---
    dlib::frontal_face_detector face_detector;
    dlib::shape_predictor landmark_predictor;

    // Only one temporary file is needed now
    std::unique_ptr<QTemporaryFile> tempModelFile;
};

#endif // BEAUTYPROCESSOR_H
