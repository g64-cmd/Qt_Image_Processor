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
// GNU General Public License for more details//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// =============================================================================

// =============================================================================
// File: gammaprocessor.cpp
//
// Description:
// GammaProcessor 类的实现文件。该文件包含了使用查找表（LUT）
// 高效实现伽马校正的算法。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "gammaprocessor.h"
#include "imageconverter.h"
#include <opencv2/opencv.hpp>

/**
 * @brief 对输入图像应用伽马校正。
 *
 * 该方法通过预先计算一个包含256个值的查找表（Look-Up Table, LUT）
 * 来实现高效的伽马校正。对于图像中的每个像素，其新值由该表直接查得，
 * 避免了对每个像素进行重复的幂函数计算。
 *
 * @param sourceImage 原始 QImage 图像。
 * @param gamma 伽马值。gamma > 1.0 会使图像变暗，gamma < 1.0 会使图像变亮。
 * @return 经过伽马校正的 QImage。
 */
QImage GammaProcessor::process(const QImage &sourceImage, double gamma)
{
    // --- 1. 输入验证 ---
    if (sourceImage.isNull() || gamma <= 0) {
        // gamma值必须为正数
        return sourceImage;
    }

    // --- 2. 格式转换 ---
    cv::Mat srcMat = ImageConverter::qImageToMat(sourceImage);
    if (srcMat.empty()) return sourceImage;

    // --- 3. 创建伽马校正查找表 (LUT) ---
    // 创建一个1行256列的矩阵来存储LUT，类型为8位无符号整数
    cv::Mat lut(1, 256, CV_8U);
    uchar* p = lut.ptr();
    double invGamma = 1.0 / gamma;

    // 遍历所有可能的像素值 (0-255)
    for (int i = 0; i < 256; ++i) {
        // 应用伽马校正公式: O = I ^ (1/gamma)
        // 首先将像素值归一化到 [0, 1]
        // 然后进行幂运算
        // 最后将结果映射回 [0, 255]
        // cv::saturate_cast<uchar> 会将结果安全地转换为0-255范围内的uchar值
        p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, invGamma) * 255.0);
    }

    // --- 4. 应用查找表 ---
    cv::Mat resultMat;
    // cv::LUT 是一个高效的函数，它会使用lut中的值来替换srcMat中对应的像素值
    cv::LUT(srcMat, lut, resultMat);

    // --- 5. 转换并返回结果 ---
    return ImageConverter::matToQImage(resultMat);
}
