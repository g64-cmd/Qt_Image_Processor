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
    static QImage textureTransfer(const QImage &contentImage, const QImage &textureImage);
    static QImage applyGamma(const QImage &sourceImage, double gamma);

    // --- 新增的色彩调整调度函数 ---
    static QImage adjustColor(const QImage &sourceImage, int brightness, int contrast, int saturation);
};

#endif // IMAGEPROCESSOR_H
