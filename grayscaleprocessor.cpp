#include "grayscaleprocessor.h"
#include "imageconverter.h"
#include <opencv2/opencv.hpp>

QImage GrayScaleProcessor::process(const QImage &sourceImage)
{
    if (sourceImage.isNull()) {
        return QImage();
    }

    // 1. 将 QImage 转换为 cv::Mat
    cv::Mat srcMat = ImageConverter::qImageToMat(sourceImage);
    if (srcMat.empty()) {
        return QImage();
    }

    // 2. 如果图像不是单通道，则转换为灰度图
    cv::Mat grayMat;
    if (srcMat.channels() == 3 || srcMat.channels() == 4) {
        cv::cvtColor(srcMat, grayMat, cv::COLOR_BGR2GRAY);
    } else {
        // 如果已经是单通道(灰度图)，直接克隆一份
        grayMat = srcMat.clone();
    }

    // 3. 将处理后的 cv::Mat 转换回 QImage 并返回
    return ImageConverter::matToQImage(grayMat);
}
