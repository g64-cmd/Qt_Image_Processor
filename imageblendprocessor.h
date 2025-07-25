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

#ifndef IMAGEBLENDPROCESSOR_H
#define IMAGEBLENDPROCESSOR_H

// =============================================================================
// File: imageblendprocessor.h
//
// Description:
// 该文件定义了 ImageBlendProcessor 类，这是一个静态工具类，
// 专门用于将两张图像按指定的权重进行线性融合。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QImage>

/**
 * @class ImageBlendProcessor
 * @brief 图像融合功能处理器。
 *
 * 遵循单一职责原则，仅负责将两张图片按权重混合。
 * 此类被设计为纯静态工具类。
 */
class ImageBlendProcessor
{
public:
    /**
     * @brief 删除默认构造函数，以防止该类的实例化。
     */
    ImageBlendProcessor() = delete;

    /**
     * @brief 对外提供的唯一处理接口，用于线性融合两张图像。
     *
     * 该函数实现了 `result = (1.0 - alpha) * imageA + alpha * imageB` 的融合逻辑。
     * @param imageA 第一张图片 (QImage)。
     * @param imageB 第二张图片 (QImage)。
     * @param alpha 图片B的权重 (0.0 to 1.0)。图片A的权重将是 (1.0 - alpha)。
     * @return 融合后的 QImage。
     */
    static QImage process(const QImage &imageA, const QImage &imageB, double alpha);
};

#endif // IMAGEBLENDPROCESSOR_H
