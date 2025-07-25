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

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

// =============================================================================
// File: imageprocessor.h
//
// Description:
// 该文件定义了 ImageProcessor 类，这是一个静态工具类，作为图像处理
// 功能的统一入口（Facade）。它整合了项目中各种独立的图像处理算法。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QImage>

/**
 * @class ImageProcessor
 * @brief 图像处理功能的外观（Facade）类。
 *
 * 这个类提供了一系列静态方法，封装了各种图像处理操作。
 * 通过使用这个类，上层代码（如MainWindow）无需与具体的处理器
 * (CannyProcessor, GammaProcessor等)直接交互，简化了调用逻辑。
 */
class ImageProcessor
{
public:
    /**
     * @brief 删除默认构造函数，以防止该类的实例化。
     */
    ImageProcessor() = delete;

    /**
     * @brief 应用锐化效果。
     * @param sourceImage 原始图像。
     * @return 锐化后的图像。
     */
    static QImage sharpen(const QImage &sourceImage);

    /**
     * @brief 将图像转换为灰度图。
     * @param sourceImage 原始图像。
     * @return 灰度图像。
     */
    static QImage grayscale(const QImage &sourceImage);

    /**
     * @brief 应用Canny边缘检测。
     * @param sourceImage 原始图像。
     * @return 只包含边缘的黑白图像。
     */
    static QImage canny(const QImage &sourceImage);

    /**
     * @brief 线性融合两张图像。
     * @param imageA 第一张图像。
     * @param imageB 第二张图像。
     * @param alpha 图像B的权重 (0.0 to 1.0)。
     * @return 融合后的图像。
     */
    static QImage blend(const QImage &imageA, const QImage &imageB, double alpha);

    /**
     * @brief 执行纹理迁移。
     * @param contentImage 内容图像。
     * @param textureImage 纹理图像。
     * @return 带有新纹理的内容图像。
     */
    static QImage textureTransfer(const QImage &contentImage, const QImage &textureImage);

    /**
     * @brief 应用伽马校正。
     * @param sourceImage 原始图像。
     * @param gamma 伽马值。
     * @return 校正后的图像。
     */
    static QImage applyGamma(const QImage &sourceImage, double gamma);

    /**
     * @brief 调整图像的色彩属性。
     * @param sourceImage 原始图像。
     * @param brightness 亮度 (-100 to 100)。
     * @param contrast 对比度 (-100 to 100)。
     * @param saturation 饱和度 (-100 to 100)。
     * @param hue 色相 (-180 to 180)。
     * @return 调整颜色后的图像。
     */
    static QImage adjustColor(const QImage &sourceImage, int brightness, int contrast, int saturation, int hue);
};

#endif // IMAGEPROCESSOR_H
