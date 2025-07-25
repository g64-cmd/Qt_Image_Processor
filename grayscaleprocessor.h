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

#ifndef GRAYSCALEPROCESSOR_H
#define GRAYSCALEPROCESSOR_H

// =============================================================================
// File: grayscaleprocessor.h
//
// Description:
// 该文件定义了 GrayScaleProcessor 类，这是一个静态工具类，
// 专门用于将彩色图像转换为灰度图像。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QImage>

/**
 * @class GrayScaleProcessor
 * @brief 灰度转换功能处理器。
 *
 * 遵循单一职责原则，封装了将图像转换为灰度图的逻辑。
 * 此类被设计为纯静态工具类。
 */
class GrayScaleProcessor
{
public:
    /**
     * @brief 删除默认构造函数，以防止该类的实例化。
     */
    GrayScaleProcessor() = delete;

    /**
     * @brief 对输入图像进行灰度转换。
     *
     * 这是该类对外提供的唯一处理接口。
     * @param sourceImage 原始 QImage 图像。
     * @return 转换后的灰度 QImage。
     */
    static QImage process(const QImage &sourceImage);
};

#endif // GRAYSCALEPROCESSOR_H
