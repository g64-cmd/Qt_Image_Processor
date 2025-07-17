#include "imageprocessor.h"
#include "imageconverter.h"
#include "grayscaleprocessor.h"
#include "cannyprocessor.h" // 确保包含了 cannyprocessor.h
#include <opencv2/opencv.hpp>

// 锐化函数的实现 (保持不变)
QImage ImageProcessor::sharpen(const QImage &sourceImage)
{
    if (sourceImage.isNull()) {
        return QImage();
    }
    cv::Mat srcMat = ImageConverter::qImageToMat(sourceImage);
    if (srcMat.empty()) {
        return QImage();
    }
    cv::Mat kernel = (cv::Mat_<float>(3, 3) <<
                          0, -1,  0,
                      -1,  5, -1,
                      0, -1,  0);
    cv::Mat dstMat;
    cv::filter2D(srcMat, dstMat, srcMat.depth(), kernel);
    return ImageConverter::matToQImage(dstMat);
}

// 灰度化函数的实现 (保持不变)
QImage ImageProcessor::grayscale(const QImage &sourceImage)
{
    return GrayScaleProcessor::process(sourceImage);
}

// Canny 边缘检测函数的实现 (这是解决问题的关键)
QImage ImageProcessor::canny(const QImage &sourceImage)
{
    // 将具体实现委托给专门的 Canny 处理器
    return CannyProcessor::process(sourceImage);
}
