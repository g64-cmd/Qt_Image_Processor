// =============================================================================
//
// Copyright (C) 2025 g64-cmd
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// =============================================================================

// =============================================================================
// File: imageprocessor.cpp
//
// Description:
// ImageProcessor 类的实现文件。该文件实现了调用各种具体图像处理
// 算法的静态方法。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "imageprocessor.h"
#include "imageconverter.h"
#include "grayscaleprocessor.h"
#include "cannyprocessor.h"
#include "imageblendprocessor.h"
#include "imagetexturetransferprocessor.h" // 假设该文件存在
#include "gammaprocessor.h"
#include "coloradjustprocessor.h"
#include <opencv2/opencv.hpp>

/**
 * @brief 应用锐化效果。
 *
 * 使用一个3x3的拉普拉斯核对图像进行2D卷积，以增强边缘，达到锐化效果。
 * @param sourceImage 原始图像。
 * @return 锐化后的图像。
 */
QImage ImageProcessor::sharpen(const QImage &sourceImage)
{
    if (sourceImage.isNull()) {
        return QImage();
    }
    cv::Mat srcMat = ImageConverter::qImageToMat(sourceImage);
    if (srcMat.empty()) {
        return QImage();
    }
    // 定义一个锐化卷积核
    cv::Mat kernel = (cv::Mat_<float>(3, 3) <<
                          0, -1,  0,
                      -1,  5, -1,
                      0, -1,  0);
    cv::Mat dstMat;
    // 应用2D滤镜
    cv::filter2D(srcMat, dstMat, srcMat.depth(), kernel);
    return ImageConverter::matToQImage(dstMat);
}

/**
 * @brief 将图像转换为灰度图。
 *
 * 直接调用 GrayScaleProcessor 的静态方法。
 * @param sourceImage 原始图像。
 * @return 灰度图像。
 */
QImage ImageProcessor::grayscale(const QImage &sourceImage)
{
    return GrayScaleProcessor::process(sourceImage);
}

/**
 * @brief 应用Canny边缘检测。
 *
 * 直接调用 CannyProcessor 的静态方法。
 * @param sourceImage 原始图像。
 * @return 只包含边缘的黑白图像。
 */
QImage ImageProcessor::canny(const QImage &sourceImage)
{
    return CannyProcessor::process(sourceImage);
}

/**
 * @brief 线性融合两张图像。
 *
 * 直接调用 ImageBlendProcessor 的静态方法。
 * @param imageA 第一张图像。
 * @param imageB 第二张图像。
 * @param alpha 图像B的权重 (0.0 to 1.0)。
 * @return 融合后的图像。
 */
QImage ImageProcessor::blend(const QImage &imageA, const QImage &imageB, double alpha)
{
    return ImageBlendProcessor::process(imageA, imageB, alpha);
}

/**
 * @brief 执行纹理迁移。
 *
 * 直接调用 ImageTextureTransferProcessor 的静态方法。
 * @param contentImage 内容图像。
 * @param textureImage 纹理图像。
 * @return 带有新纹理的内容图像。
 */
QImage ImageProcessor::textureTransfer(const QImage &contentImage, const QImage &textureImage)
{
    // 假设 ImageTextureTransferProcessor::process 存在
    return ImageTextureTransferProcessor::process(contentImage, textureImage);
}

/**
 * @brief 应用伽马校正。
 *
 * 直接调用 GammaProcessor 的静态方法。
 * @param sourceImage 原始图像。
 * @param gamma 伽马值。
 * @return 校正后的图像。
 */
QImage ImageProcessor::applyGamma(const QImage &sourceImage, double gamma)
{
    return GammaProcessor::process(sourceImage, gamma);
}

/**
 * @brief 调整图像的色彩属性。
 *
 * 这是一个两步过程：首先调整亮度和对比度，然后对结果图像调整饱和度和色相。
 * @param sourceImage 原始图像。
 * @param brightness 亮度 (-100 to 100)。
 * @param contrast 对比度 (-100 to 100)。
 * @param saturation 饱和度 (-100 to 100)。
 * @param hue 色相 (-180 to 180)。
 * @return 调整颜色后的图像。
 */
QImage ImageProcessor::adjustColor(const QImage &sourceImage, int brightness, int contrast, int saturation, int hue)
{
    // 步骤1: 调整亮度和对比度
    QImage tempImage = ColorAdjustProcessor::adjustBrightnessContrast(sourceImage, brightness, contrast);
    // 步骤2: 在上一步结果的基础上，调整饱和度和色相
    return ColorAdjustProcessor::adjustSaturationHue(tempImage, saturation, hue);
}
