#ifndef CANNYPROCESSOR_H
#define CANNYPROCESSOR_H

#include <QImage>

/**
 * @brief Canny 边缘检测功能处理器
 *
 * 遵循单一职责原则，仅负责执行 Canny 边缘检测算法。
 */
class CannyProcessor
{
public:
    // 这是一个纯静态工具类，禁止实例化
    CannyProcessor() = delete;

    /**
     * @brief 对外提供的唯一处理接口
     * @param sourceImage 原始 QImage
     * @return 只包含边缘信息的黑白 QImage
     */
    static QImage process(const QImage &sourceImage);
};

#endif // CANNYPROCESSOR_H
