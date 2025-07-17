#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QImage>

/**
 * @brief 图像处理核心类
 *
 * 包含所有静态的图像处理算法函数。
 */
class ImageProcessor
{
public:
    ImageProcessor() = delete; // 禁止实例化

    /**
     * @brief 对图像进行锐化处理
     * @param sourceImage 原始 QImage
     * @return 锐化后的 QImage
     */
    static QImage sharpen(const QImage &sourceImage);

    /**
     * @brief 将图像转换为灰度图
     * @param sourceImage 原始 QImage
     * @return 灰度化后的 QImage
     */
    static QImage grayscale(const QImage &sourceImage);

    // 未来可以继续在这里添加其他静态处理函数
    // static QImage applyGamma(const QImage &sourceImage, double gamma);
};

#endif // IMAGEPROCESSOR_H
