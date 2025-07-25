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

#ifndef COLORADJUSTPROCESSOR_H
#define COLORADJUSTPROCESSOR_H

// =============================================================================
// File: coloradjustprocessor.h
//
// Description:
// 该文件定义了 ColorAdjustProcessor 类，这是一个静态工具类，
// 负责执行图像的色彩调整，包括亮度、对比度、饱和度和色相。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QImage>

/**
 * @class ColorAdjustProcessor
 * @brief 图像色彩调整功能处理器。
 *
 * 封装了亮度、对比度、饱和度和色相的调整算法。
 * 此类被设计为纯静态工具类，所有方法都应通过类名直接调用。
 */
class ColorAdjustProcessor
{
public:
    /**
     * @brief 删除默认构造函数，以防止该类的实例化。
     */
    ColorAdjustProcessor() = delete;

    /**
     * @brief 调整图像的亮度和对比度。
     * @param sourceImage 原始 QImage 图像。
     * @param brightness 亮度调整值 (-100 to 100)。正值增加亮度，负值降低亮度。
     * @param contrast 对比度调整值 (-100 to 100)。正值增加对比度，负值降低对比度。
     * @return 调整后的 QImage。
     */
    static QImage adjustBrightnessContrast(const QImage &sourceImage, int brightness, int contrast);

    /**
     * @brief 调整图像的饱和度和色相。
     * @param sourceImage 原始 QImage 图像。
     * @param saturation 饱和度调整值 (-100 to 100)。正值增加饱和度，负值降低饱和度。
     * @param hue 色相偏移值 (-180 to 180)。表示色轮上的旋转角度。
     * @return 调整后的 QImage。
     */
    static QImage adjustSaturationHue(const QImage &sourceImage, int saturation, int hue);
};

#endif // COLORADJUSTPROCESSOR_H
