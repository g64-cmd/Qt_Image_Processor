#include "imageprocessor.h"
#include "imageconverter.h"
#include "grayscaleprocessor.h"
#include "cannyprocessor.h"
#include "imageblendprocessor.h"
#include "imagetexturetransferprocessor.h"
#include "gammaprocessor.h"
#include "coloradjustprocessor.h"
#include <opencv2/opencv.hpp>

// ... (其他函数保持不变) ...
QImage ImageProcessor::sharpen(const QImage &sourceImage){if(sourceImage.isNull()){return QImage();}cv::Mat srcMat=ImageConverter::qImageToMat(sourceImage);if(srcMat.empty()){return QImage();}cv::Mat kernel=(cv::Mat_<float>(3,3)<<0,-1,0,-1,5,-1,0,-1,0);cv::Mat dstMat;cv::filter2D(srcMat,dstMat,srcMat.depth(),kernel);return ImageConverter::matToQImage(dstMat);}
QImage ImageProcessor::grayscale(const QImage &sourceImage){return GrayScaleProcessor::process(sourceImage);}
QImage ImageProcessor::canny(const QImage &sourceImage){return CannyProcessor::process(sourceImage);}
QImage ImageProcessor::blend(const QImage &imageA, const QImage &imageB, double alpha){return ImageBlendProcessor::process(imageA,imageB,alpha);}
QImage ImageProcessor::textureTransfer(const QImage &contentImage, const QImage &textureImage){return ImageTextureTransferProcessor::process(contentImage,textureImage);}
QImage ImageProcessor::applyGamma(const QImage &sourceImage, double gamma){return GammaProcessor::process(sourceImage,gamma);}


// --- 更新色彩调整调度函数 ---
QImage ImageProcessor::adjustColor(const QImage &sourceImage, int brightness, int contrast, int saturation, int hue)
{
    // 顺序很重要：先调整亮度和对比度，再调整饱和度和色相
    QImage tempImage = ColorAdjustProcessor::adjustBrightnessContrast(sourceImage, brightness, contrast);
    return ColorAdjustProcessor::adjustSaturationHue(tempImage, saturation, hue);
}
