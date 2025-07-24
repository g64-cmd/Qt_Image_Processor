#include "grayscaleprocessor.h"
#include "imageconverter.h"
#include <opencv2/opencv.hpp>

QImage GrayScaleProcessor::process(const QImage &sourceImage)
{
    if (sourceImage.isNull()) {
        return QImage();
    }
    cv::Mat srcMat = ImageConverter::qImageToMat(sourceImage);
    if (srcMat.empty()) {
        return QImage();
    }
    cv::Mat grayMat;
    if (srcMat.channels() == 3 || srcMat.channels() == 4) {
        cv::cvtColor(srcMat, grayMat, cv::COLOR_BGR2GRAY);
    } else {
        grayMat = srcMat.clone();
    }
    return ImageConverter::matToQImage(grayMat);
}
