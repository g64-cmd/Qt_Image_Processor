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

#ifndef CANNYPROCESSOR_H
#define CANNYPROCESSOR_H

// =============================================================================
// File: cannyprocessor.h
//
// Description:
// 该文件定义了 CannyProcessor 类，这是一个静态工具类，
// 专门用于执行 Canny 边缘检测算法。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QImage>

/**
 * @class CannyProcessor
 * @brief Canny 边缘检测功能处理器。
 *
 * 遵循单一职责原则，此类封装了执行 Canny 边缘检测的所有逻辑。
 * 它被设计为一个纯静态工具类，所有功能通过静态方法调用。
 */
class CannyProcessor
{
public:
    /**
     * @brief 删除默认构造函数，以防止该类的实例化。
     *
     * 这是一个纯静态工具类，不应该创建任何实例。
     */
    CannyProcessor() = delete;

    /**
     * @brief 对输入图像应用 Canny 边缘检测算法。
     *
     * 这是该类对外提供的唯一处理接口。它接收一个QImage，
     * 在内部将其转换为OpenCV格式，应用Canny算法，然后将结果
     * 转换回QImage。
     *
     * @param sourceImage 待处理的原始 QImage 图像。
     * @return 返回一个只包含边缘信息的黑白 QImage。
     */
    static QImage process(const QImage &sourceImage);
};

#endif // CANNYPROCESSOR_H
