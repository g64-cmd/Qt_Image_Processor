#include "cannyprocessor.h"
#include "imageconverter.h"
#include <opencv2/opencv.hpp>

QImage CannyProcessor::process(const QImage &sourceImage)
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
    cv::GaussianBlur(grayMat, grayMat, cv::Size(3, 3), 0);
    cv::Mat edgesMat;
    cv::Canny(grayMat, edgesMat, 50, 150);
    return ImageConverter::matToQImage(edgesMat);
}
