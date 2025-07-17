#include "imageprocessor.h"
#include "imageconverter.h"
#include "grayscaleprocessor.h" // <<< 1. 包含新的灰度化处理器头文件
#include <opencv2/opencv.hpp>

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

/**
 * @brief 将图像转换为灰度图 (调度)
 * @param sourceImage 原始 QImage
 * @return 灰度化后的 QImage
 */
QImage ImageProcessor::grayscale(const QImage &sourceImage)
{
    // <<< 2. 将具体实现委托给专门的处理器
    return GrayScaleProcessor::process(sourceImage);
}
