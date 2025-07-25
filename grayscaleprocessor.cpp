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
// File: grayscaleprocessor.cpp
//
// Description:
// GrayScaleProcessor 类的实现文件。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "grayscaleprocessor.h"
#include "imageconverter.h"
#include <opencv2/opencv.hpp>

/**
 * @brief 对输入图像进行灰度转换。
 *
 * 算法流程如下：
 * 1. 将输入的 QImage 转换为 cv::Mat。
 * 2. 如果图像是彩色的（3或4通道），则使用 cv::cvtColor 将其转换为灰度图。
 * 3. 如果图像已经是单通道的，则直接使用。
 * 4. 将结果转换回 QImage 并返回。
 *
 * @param sourceImage 待处理的原始 QImage 图像。
 * @return 转换后的灰度 QImage。如果输入无效，则返回一个空的QImage。
 */
QImage GrayScaleProcessor::process(const QImage &sourceImage)
{
    // --- 1. 输入验证 ---
    if (sourceImage.isNull()) {
        return QImage();
    }

    // --- 2. 格式转换 ---
    cv::Mat srcMat = ImageConverter::qImageToMat(sourceImage);
    if (srcMat.empty()) {
        return QImage();
    }

    // --- 3. 灰度转换 ---
    cv::Mat grayMat;
    // 检查通道数，如果是彩色图(BGR或BGRA)则转为灰度图
    if (srcMat.channels() == 3 || srcMat.channels() == 4) {
        cv::cvtColor(srcMat, grayMat, cv::COLOR_BGR2GRAY);
    } else {
        // 如果已经是单通道图，则直接克隆使用
        grayMat = srcMat.clone();
    }

    // --- 4. 转换并返回结果 ---
    return ImageConverter::matToQImage(grayMat);
}
