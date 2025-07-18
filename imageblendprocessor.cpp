#include "imageblendprocessor.h"
#include "imageconverter.h"
#include <opencv2/opencv.hpp>

QImage ImageBlendProcessor::process(const QImage &imageA, const QImage &imageB, double alpha)
{
    if (imageA.isNull() || imageB.isNull()) {
        return QImage();
    }

    // 1. 将 QImage 转换为 cv::Mat
    cv::Mat matA = ImageConverter::qImageToMat(imageA);
    cv::Mat matB = ImageConverter::qImageToMat(imageB);

    if (matA.empty() || matB.empty()) {
        return QImage();
    }

    // 2. 关键步骤：确保两张图片尺寸和类型完全一致，以便进行像素级操作
    if (matA.size() != matB.size()) {
        cv::resize(matB, matB, matA.size());
    }
    if (matA.type() != matB.type()) {
        matB.convertTo(matB, matA.type());
    }
    // 确保通道数也一致
    if (matA.channels() != matB.channels()) {
        int code = (matA.channels() == 3) ? cv::COLOR_BGRA2BGR : cv::COLOR_BGR2BGRA;
        if (matA.channels() > matB.channels()) {
            // 这种情况很少见，但为了稳健性还是处理一下
        } else {
            cv::cvtColor(matB, matB, code);
        }
    }


    // 3. 使用 OpenCV 的 addWeighted 函数进行线性混合
    cv::Mat resultMat;
    double beta = 1.0 - alpha; // 图片A的权重
    cv::addWeighted(matA, beta, matB, alpha, 0.0, resultMat);

    // 4. 将结果转换回 QImage 并返回
    return ImageConverter::matToQImage(resultMat);
}
