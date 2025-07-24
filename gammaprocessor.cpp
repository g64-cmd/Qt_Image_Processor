#include "gammaprocessor.h"
#include "imageconverter.h"
#include <opencv2/opencv.hpp>

QImage GammaProcessor::process(const QImage &sourceImage, double gamma)
{
    if (sourceImage.isNull() || gamma <= 0) {
        return sourceImage;
    }
    cv::Mat srcMat = ImageConverter::qImageToMat(sourceImage);
    if (srcMat.empty()) return sourceImage;
    cv::Mat lut(1, 256, CV_8U);
    uchar* p = lut.ptr();
    double invGamma = 1.0 / gamma;
    for (int i = 0; i < 256; ++i) {
        p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, invGamma) * 255.0);
    }
    cv::Mat resultMat;
    cv::LUT(srcMat, lut, resultMat);
    return ImageConverter::matToQImage(resultMat);
}
