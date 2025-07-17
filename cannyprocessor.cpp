#include "cannyprocessor.h"
#include "imageconverter.h"
#include <opencv2/opencv.hpp>

QImage CannyProcessor::process(const QImage &sourceImage)
{
    if (sourceImage.isNull()) {
        return QImage();
    }

    // 1. 将 QImage 转换为 cv::Mat
    cv::Mat srcMat = ImageConverter::qImageToMat(sourceImage);
    if (srcMat.empty()) {
        return QImage();
    }

    // 2. 将源图像转换为灰度图，因为 Canny 算法需要单通道图像
    cv::Mat grayMat;
    if (srcMat.channels() == 3 || srcMat.channels() == 4) {
        cv::cvtColor(srcMat, grayMat, cv::COLOR_BGR2GRAY);
    } else {
        grayMat = srcMat.clone();
    }

    // 3. (可选) 应用高斯模糊以减少噪声，提高检测效果
    cv::GaussianBlur(grayMat, grayMat, cv::Size(3, 3), 0);

    // 4. 执行 Canny 边缘检测
    //    参数 50 和 150 是低阈值和高阈值，可以根据需要调整
    cv::Mat edgesMat;
    cv::Canny(grayMat, edgesMat, 50, 150);

    // 5. 将处理后的单通道边缘图像转换回 QImage 并返回
    return ImageConverter::matToQImage(edgesMat);
}
