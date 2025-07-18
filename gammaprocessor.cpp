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

    // 1. 构建一个查找表 (Look-Up Table, LUT)
    // 这样我们只需要计算256次，而不是对每个像素都进行幂运算，效率极高。
    cv::Mat lut(1, 256, CV_8U);
    uchar* p = lut.ptr();
    double invGamma = 1.0 / gamma;
    for (int i = 0; i < 256; ++i) {
        // 伽马校正公式，并确保结果在 0-255 范围内
        p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, invGamma) * 255.0);
    }

    // 2. 使用 LUT 应用变换
    cv::Mat resultMat;
    cv::LUT(srcMat, lut, resultMat);

    // 3. 将结果转换回 QImage 并返回
    return ImageConverter::matToQImage(resultMat);
}
