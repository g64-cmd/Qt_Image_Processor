#include "beautyprocessor.h"
#include "imageconverter.h"

QImage BeautyProcessor::process(const QImage &sourceImage, int smoothLevel)
{
    if (sourceImage.isNull()) {
        return sourceImage;
    }

    cv::Mat originalMat = ImageConverter::qImageToMat(sourceImage);
    if (originalMat.empty()) return sourceImage;

    // --- 关键修复：确保图像是3通道的 ---
    // bilateralFilter 不支持带有 Alpha 通道 (4通道) 的图像。
    // 在处理前，我们检查并将其转换为标准的3通道BGR图像。
    if (originalMat.channels() == 4) {
        cv::cvtColor(originalMat, originalMat, cv::COLOR_BGRA2BGR);
    }

    // 创建一个副本用于处理
    cv::Mat processedMat = originalMat.clone();

    // 应用磨皮
    if (smoothLevel > 0) {
        applySkinSmoothing(processedMat, smoothLevel);
    }

    return ImageConverter::matToQImage(processedMat);
}

void BeautyProcessor::applySkinSmoothing(cv::Mat &image, int level)
{
    if (level <= 0) return;

    // 磨皮等级越高，滤波器的参数d和sigmaColor就越大
    int d = level / 10 * 2 + 1; // 确保 d 是一个正奇数
    double sigmaColor = level * 0.5;
    double sigmaSpace = level * 0.5;

    cv::Mat temp;
    // 使用标准库中的双边滤波器
    cv::bilateralFilter(image, temp, d, sigmaColor, sigmaSpace);
    // 应用两次以获得更平滑的效果
    cv::bilateralFilter(temp, image, d, sigmaColor, sigmaSpace);
}
