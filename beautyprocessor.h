#ifndef BEAUTYPROCESSOR_H
#define BEAUTYPROCESSOR_H

#include <QImage>
#include <opencv2/opencv.hpp> // 只需要标准库

/**
 * @brief 图像美颜功能核心处理器 (仅包含标准库功能)
 *
 * 封装了基于双边滤波的磨皮算法。
 */
class BeautyProcessor
{
public:
    BeautyProcessor() = default; // 无需加载任何模型

    /**
     * @brief 对外提供的唯一处理接口
     * @param sourceImage 原始 QImage
     * @param smoothLevel 磨皮等级 (0-100)
     * @return 美颜后的 QImage
     */
    QImage process(const QImage &sourceImage, int smoothLevel);

private:
    void applySkinSmoothing(cv::Mat &image, int level);
};

#endif // BEAUTYPROCESSOR_H
