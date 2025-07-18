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
    static QImage blend(const QImage &imageA, const QImage &imageB, double alpha);

    // --- 新增的纹理迁移调度函数 ---
    static QImage textureTransfer(const QImage &contentImage, const QImage &textureImage);
};

#endif // IMAGEPROCESSOR_H
