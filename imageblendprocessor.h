#ifndef IMAGEBLENDPROCESSOR_H
#define IMAGEBLENDPROCESSOR_H

#include <QImage>

/**
 * @brief 图像融合功能处理器
 *
 * 遵循单一职责原则，仅负责将两张图片按权重混合。
 */
class ImageBlendProcessor
{
public:
    ImageBlendProcessor() = delete;

    /**
     * @brief 对外提供的唯一处理接口
     * @param imageA 第一张图片 (QImage)
     * @param imageB 第二张图片 (QImage)
     * @param alpha 图片B的权重 (0.0 to 1.0)。图片A的权重将是 (1.0 - alpha)。
     * @return 融合后的 QImage
     */
    static QImage process(const QImage &imageA, const QImage &imageB, double alpha);
};

#endif // IMAGEBLENDPROCESSOR_H
