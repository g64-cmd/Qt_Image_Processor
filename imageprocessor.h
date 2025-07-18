#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QImage>

class ImageProcessor
{
public:
    ImageProcessor() = delete;

    static QImage sharpen(const QImage &sourceImage);
    static QImage grayscale(const QImage &sourceImage);
    static QImage canny(const QImage &sourceImage);

    // --- 新增的融合调度函数 ---
    static QImage blend(const QImage &imageA, const QImage &imageB, double alpha);
};

#endif // IMAGEPROCESSOR_H
