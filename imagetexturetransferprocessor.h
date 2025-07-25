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

#ifndef IMAGETEXTURETRANSFERPROCESSOR_H
#define IMAGETEXTURETRANSFERPROCESSOR_H

// =============================================================================
// File: imagetexturetransferprocessor.h
//
// Description:
// 该文件定义了 ImageTextureTransferProcessor 类，这是一个静态工具类，
// 负责执行图像的纹理迁移（或风格迁移）算法。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QImage>

/**
 * @class ImageTextureTransferProcessor
 * @brief 图像纹理迁移功能处理器。
 *
 * 遵循单一职责原则，使用经典的颜色统计量匹配算法，将一张图片的
 * 纹理（风格）应用到另一张图片的内容上。此类被设计为纯静态工具类。
 */
class ImageTextureTransferProcessor
{
public:
    /**
     * @brief 删除默认构造函数，以防止该类的实例化。
     */
    ImageTextureTransferProcessor() = delete;

    /**
     * @brief 对外提供的唯一处理接口。
     * @param contentImage 内容图片 (QImage)。
     * @param textureImage 纹理/风格图片 (QImage)。
     * @return 迁移了纹理的新 QImage。
     */
    static QImage process(const QImage &contentImage, const QImage &textureImage);
};

#endif // IMAGETEXTURETRANSFERPROCESSOR_H
