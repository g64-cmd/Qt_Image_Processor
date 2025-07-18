#include "imageprocessor.h"
#include "imageconverter.h"
#include "grayscaleprocessor.h"
#include "cannyprocessor.h"
#include "imageblendprocessor.h" // <-- 1. 包含新的融合处理器头文件
#include <opencv2/opencv.hpp>

QImage ImageProcessor::sharpen(const QImage &sourceImage)
{
    // ... (代码不变)
    if (sourceImage.isNull()) { return QImage(); }
    cv::Mat srcMat = ImageConverter::qImageToMat(sourceImage);
    if (srcMat.empty()) { return QImage(); }
    cv::Mat kernel = (cv::Mat_<float>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
    cv::Mat dstMat;
    cv::filter2D(srcMat, dstMat, srcMat.depth(), kernel);
    return ImageConverter::matToQImage(dstMat);
}

QImage ImageProcessor::grayscale(const QImage &sourceImage)
{
    return GrayScaleProcessor::process(sourceImage);
}

QImage ImageProcessor::canny(const QImage &sourceImage)
{
    return CannyProcessor::process(sourceImage);
}

// --- 2. 实现新的融合调度函数 ---
QImage ImageProcessor::blend(const QImage &imageA, const QImage &imageB, double alpha)
{
    // 将具体实现委托给专门的处理器
    return ImageBlendProcessor::process(imageA, imageB, alpha);
}
