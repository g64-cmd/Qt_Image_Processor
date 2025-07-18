#ifndef GAMMAPROCESSOR_H
#define GAMMAPROCESSOR_H

#include <QImage>

/**
 * @brief 伽马变换功能处理器
 *
 * 遵循单一职责原则，使用查找表 (LUT) 高效地应用伽马校正。
 */
class GammaProcessor
{
public:
    GammaProcessor() = delete;

    /**
     * @brief 对外提供的唯一处理接口
     * @param sourceImage 原始 QImage
     * @param gamma 伽马值 (例如: 1.0 代表不变, <1.0 变亮, >1.0 变暗)
     * @return 经过伽马校正的 QImage
     */
    static QImage process(const QImage &sourceImage, double gamma);
};

#endif // GAMMAPROCESSOR_H
