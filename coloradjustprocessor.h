#ifndef COLORADJUSTPROCESSOR_H
#define COLORADJUSTPROCESSOR_H

#include <QImage>

/**
 * @brief 图像色彩调整功能处理器
 *
 * 封装了亮度、对比度和饱和度的调整算法。
 */
class ColorAdjustProcessor
{
public:
    ColorAdjustProcessor() = delete;

    /**
     * @brief 调整图像的亮度和对比度
     * @param sourceImage 原始 QImage
     * @param brightness 亮度值 (-100 to 100)
     * @param contrast 对比度值 (-100 to 100)
     * @return 调整后的 QImage
     */
    static QImage adjustBrightnessContrast(const QImage &sourceImage, int brightness, int contrast);

    /**
     * @brief 调整图像的饱和度
     * @param sourceImage 原始 QImage
     * @param saturation 饱和度值 (-100 to 100)
     * @return 调整后的 QImage
     */
    static QImage adjustSaturation(const QImage &sourceImage, int saturation);
};

#endif // COLORADJUSTPROCESSOR_H
