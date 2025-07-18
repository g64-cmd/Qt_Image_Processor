#include "coloradjustprocessor.h"
#include "imageconverter.h"
#include <opencv2/opencv.hpp>

QImage ColorAdjustProcessor::adjustBrightnessContrast(const QImage &sourceImage, int brightness, int contrast)
{
    if (sourceImage.isNull()) {
        return sourceImage;
    }

    cv::Mat srcMat = ImageConverter::qImageToMat(sourceImage);
    if (srcMat.empty()) return sourceImage;

    // 将 contrast (-100 to 100) 映射到 alpha (0.0 to 2.0)
    double alpha = 1.0 + contrast / 100.0;
    // 将 brightness (-100 to 100) 映射到 beta (-100 to 100)
    double beta = brightness;

    cv::Mat resultMat;
    srcMat.convertTo(resultMat, -1, alpha, beta);

    return ImageConverter::matToQImage(resultMat);
}

QImage ColorAdjustProcessor::adjustSaturation(const QImage &sourceImage, int saturation)
{
    if (sourceImage.isNull() || saturation == 0) {
        return sourceImage;
    }

    cv::Mat srcMat = ImageConverter::qImageToMat(sourceImage);
    if (srcMat.empty()) return sourceImage;

    cv::Mat hsvMat;
    cv::cvtColor(srcMat, hsvMat, cv::COLOR_BGR2HSV);

    std::vector<cv::Mat> hsvChannels;
    cv::split(hsvMat, hsvChannels);

    // 将 saturation (-100 to 100) 映射到增益 (0.0 to 2.0)
    double gain = 1.0 + saturation / 100.0;

    // 对饱和度通道应用增益
    hsvChannels[1].convertTo(hsvChannels[1], CV_64F);
    hsvChannels[1] = hsvChannels[1] * gain;
    // 确保值在 0-255 范围内
    cv::threshold(hsvChannels[1], hsvChannels[1], 255, 255, cv::THRESH_TRUNC);
    hsvChannels[1].convertTo(hsvChannels[1], CV_8U);

    cv::merge(hsvChannels, hsvMat);

    cv::Mat resultMat;
    cv::cvtColor(hsvMat, resultMat, cv::COLOR_HSV2BGR);

    return ImageConverter::matToQImage(resultMat);
}
