#ifndef COLORADJUSTPROCESSOR_H
#define COLORADJUSTPROCESSOR_H

#include <QImage>

/**
 * @brief 图像色彩调整功能处理器
 *
 * 封装了亮度、对比度、饱和度和色相的调整算法。
 */
class ColorAdjustProcessor
{
public:
    ColorAdjustProcessor() = delete;

    /**
     * @brief 调整图像的亮度和对比度
     */
    static QImage adjustBrightnessContrast(const QImage &sourceImage, int brightness, int contrast);

    /**
     * @brief 调整图像的饱和度和色相
     * @param sourceImage 原始 QImage
     * @param saturation 饱和度值 (-100 to 100)
     * @param hue 色相偏移值 (-180 to 180)
     * @return 调整后的 QImage
     */
    static QImage adjustSaturationHue(const QImage &sourceImage, int saturation, int hue);
};

#endif // COLORADJUSTPROCESSOR_H
