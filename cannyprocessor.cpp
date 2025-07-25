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
// File: cannyprocessor.cpp
//
// Description:
// CannyProcessor 类的实现文件。该文件包含了执行 Canny 边缘检测
// 算法的具体步骤。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "cannyprocessor.h"
#include "imageconverter.h"
#include <opencv2/opencv.hpp>

/**
 * @brief 对输入图像应用 Canny 边缘检测算法。
 *
 * 算法流程如下：
 * 1. 将输入的 QImage 转换为 cv::Mat。
 * 2. 将图像转换为灰度图。
 * 3. 应用高斯模糊以减少噪声，为边缘检测做准备。
 * 4. 使用 cv::Canny 函数执行边缘检测。
 * 5. 将结果转换回 QImage 并返回。
 *
 * @param sourceImage 待处理的原始 QImage 图像。
 * @return 返回一个只包含边缘信息的黑白 QImage。如果输入无效，则返回一个空的QImage。
 */
QImage CannyProcessor::process(const QImage &sourceImage)
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
    // 检查通道数，如果是彩色图则转为灰度图
    if (srcMat.channels() == 3 || srcMat.channels() == 4) {
        cv::cvtColor(srcMat, grayMat, cv::COLOR_BGR2GRAY);
    } else {
        // 如果已经是单通道图，则直接使用
        grayMat = srcMat.clone();
    }

    // --- 4. 预处理：高斯模糊 ---
    // 在边缘检测之前应用高斯模糊可以减少图像中的噪声，避免检测到伪边缘。
    cv::GaussianBlur(grayMat, grayMat, cv::Size(3, 3), 0);

    // --- 5. 执行 Canny 边缘检测 ---
    cv::Mat edgesMat;
    // 50 和 150 是低阈值和高阈值，用于连接边缘。
    cv::Canny(grayMat, edgesMat, 50, 150);

    // --- 6. 转换并返回结果 ---
    return ImageConverter::matToQImage(edgesMat);
}
