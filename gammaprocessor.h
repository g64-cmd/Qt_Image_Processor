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

#ifndef GAMMAPROCESSOR_H
#define GAMMAPROCESSOR_H

// =============================================================================
// File: gammaprocessor.h
//
// Description:
// 该文件定义了 GammaProcessor 类，这是一个静态工具类，
// 专门用于对图像执行伽马校正。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QImage>

/**
 * @class GammaProcessor
 * @brief 伽马变换功能处理器。
 *
 * 遵循单一职责原则，使用查找表 (LUT) 高效地应用伽马校正。
 * 此类被设计为纯静态工具类。
 */
class GammaProcessor
{
public:
    /**
     * @brief 删除默认构造函数，以防止该类的实例化。
     */
    GammaProcessor() = delete;

    /**
     * @brief 对输入图像应用伽马校正。
     *
     * 这是该类对外提供的唯一处理接口。它根据给定的伽马值，
     * 计算一个查找表（LUT），然后将此变换应用于图像的每个像素。
     *
     * @param sourceImage 原始 QImage 图像。
     * @param gamma 伽马值 (例如: 1.0 代表不变, <1.0 变亮, >1.0 变暗)。
     * @return 经过伽马校正的 QImage。
     */
    static QImage process(const QImage &sourceImage, double gamma);
};

#endif // GAMMAPROCESSOR_H
