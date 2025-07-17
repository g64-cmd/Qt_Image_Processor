#include "imageprocessor.h"
#include "imageconverter.h" // 引入我们刚刚创建的转换器
#include <opencv2/opencv.hpp>

QImage ImageProcessor::sharpen(const QImage &sourceImage)
{
    if (sourceImage.isNull()) {
        return QImage();
    }

    // 1. 将 QImage 转换为 cv::Mat
    cv::Mat srcMat = ImageConverter::qImageToMat(sourceImage);
    if (srcMat.empty()) {
        return QImage();
    }

    // 2. 定义锐化核 (Laplacian)
    //    | 0 -1  0 |
    //    |-1  5 -1 |
    //    | 0 -1  0 |
    cv::Mat kernel = (cv::Mat_<float>(3, 3) <<
                          0, -1,  0,
                      -1,  5, -1,
                      0, -1,  0);

    // 3. 应用 2D 滤波器
    cv::Mat dstMat;
    cv::filter2D(srcMat, dstMat, srcMat.depth(), kernel);

    // 4. 将处理后的 cv::Mat 转换回 QImage 并返回
    return ImageConverter::matToQImage(dstMat);
}
