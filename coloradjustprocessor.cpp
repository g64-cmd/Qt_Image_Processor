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
    double alpha = 1.0 + contrast / 100.0;
    double beta = brightness;
    cv::Mat resultMat;
    srcMat.convertTo(resultMat, -1, alpha, beta);
    return ImageConverter::matToQImage(resultMat);
}

QImage ColorAdjustProcessor::adjustSaturationHue(const QImage &sourceImage, int saturation, int hue)
{
    if (sourceImage.isNull()) {
        return sourceImage;
    }
    cv::Mat srcMat = ImageConverter::qImageToMat(sourceImage);
    if (srcMat.empty()) return sourceImage;
    cv::Mat hsvMat;
    cv::cvtColor(srcMat, hsvMat, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> hsvChannels;
    cv::split(hsvMat, hsvChannels);
    if (saturation != 0) {
        double satGain = 1.0 + saturation / 100.0;
        hsvChannels[1].convertTo(hsvChannels[1], CV_64F);
        hsvChannels[1] = hsvChannels[1] * satGain;
        cv::threshold(hsvChannels[1], hsvChannels[1], 255, 255, cv::THRESH_TRUNC);
        hsvChannels[1].convertTo(hsvChannels[1], CV_8U);
    }
    if (hue != 0) {
        hsvChannels[0].convertTo(hsvChannels[0], CV_32S);
        for(int i = 0; i < hsvChannels[0].rows; ++i) {
            for(int j = 0; j < hsvChannels[0].cols; ++j) {
                int &pixel = hsvChannels[0].at<int>(i,j);
                pixel = (pixel + hue) % 180;
                if(pixel < 0) {
                    pixel += 180;
                }
            }
        }
        hsvChannels[0].convertTo(hsvChannels[0], CV_8U);
    }
    cv::merge(hsvChannels, hsvMat);
    cv::Mat resultMat;
    cv::cvtColor(hsvMat, resultMat, cv::COLOR_HSV2BGR);
    return ImageConverter::matToQImage(resultMat);
}
