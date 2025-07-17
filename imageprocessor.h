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
     */
    static QImage sharpen(const QImage &sourceImage);

    /**
     * @brief 将图像转换为灰度图
     */
    static QImage grayscale(const QImage &sourceImage);

    /**
     * @brief 对图像进行 Canny 边缘检测
     */
    static QImage canny(const QImage &sourceImage);
};

#endif // IMAGEPROCESSOR_H
