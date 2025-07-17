#ifndef GRAYSCALEPROCESSOR_H
#define GRAYSCALEPROCESSOR_H

#include <QImage>

/**
 * @brief 灰度化功能处理器
 *
 * 遵循单一职责原则，仅负责将图像转换为灰度图。
 */
class GrayScaleProcessor
{
public:
    // 这是一个纯静态工具类，禁止实例化
    GrayScaleProcessor() = delete;

    /**
     * @brief 对外提供的唯一处理接口
     * @param sourceImage 原始 QImage
     * @return 灰度化后的 QImage
     */
    static QImage process(const QImage &sourceImage);
};

#endif // GRAYSCALEPROCESSOR_H
