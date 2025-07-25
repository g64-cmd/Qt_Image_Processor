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
// File: imageblendprocessor.cpp
//
// Description:
// ImageBlendProcessor 类的实现文件。该文件包含了使用OpenCV的
// addWeighted函数实现图像线性融合的逻辑。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "imageblendprocessor.h"
#include "imageconverter.h"
#include <opencv2/opencv.hpp>

/**
 * @brief 对外提供的唯一处理接口，用于线性融合两张图像。
 *
 * 该函数使用OpenCV的 `cv::addWeighted` 函数来高效地执行图像融合。
 * 在融合前，会进行一系列预处理，确保两张图像的尺寸、类型和通道数一致。
 *
 * @param imageA 第一张图片 (QImage)。
 * @param imageB 第二张图片 (QImage)。
 * @param alpha 图片B的权重 (0.0 to 1.0)。图片A的权重将是 (1.0 - alpha)。
 * @return 融合后的 QImage。如果输入无效，则返回一个空的QImage。
 */
QImage ImageBlendProcessor::process(const QImage &imageA, const QImage &imageB, double alpha)
{
    // --- 1. 输入验证 ---
    if (imageA.isNull() || imageB.isNull()) {
        return QImage();
    }

    // --- 2. 格式转换 ---
    cv::Mat matA = ImageConverter::qImageToMat(imageA);
    cv::Mat matB = ImageConverter::qImageToMat(imageB);
    if (matA.empty() || matB.empty()) {
        return QImage();
    }

    // --- 3. 预处理：确保图像属性一致 ---
    // cv::addWeighted 要求两张输入图像必须具有相同的尺寸和类型。

    // a. 检查并统一尺寸
    if (matA.size() != matB.size()) {
        // 如果尺寸不匹配，将图像B的大小调整为与图像A相同
        cv::resize(matB, matB, matA.size());
    }

    // b. 检查并统一类型
    if (matA.type() != matB.type()) {
        matB.convertTo(matB, matA.type());
    }

    // c. 检查并统一通道数
    if (matA.channels() != matB.channels()) {
        // 这个逻辑块处理一个3通道和一个4通道图像的情况
        int code = (matA.channels() == 3) ? cv::COLOR_BGRA2BGR : cv::COLOR_BGR2BGRA;
        if (matA.channels() > matB.channels()) {
            // 如果A的通道数多于B（例如A是4通道，B是3通道），此处未作处理
        } else {
            // 如果B的通道数多于A，则转换B以匹配A
            cv::cvtColor(matB, matB, code);
        }
    }

    // --- 4. 执行加权融合 ---
    cv::Mat resultMat;
    // 计算图像A的权重
    double beta = 1.0 - alpha;
    // cv::addWeighted 执行公式: result = matA * beta + matB * alpha + gamma
    // 这里的 gamma (最后一个参数) 设置为0.0
    cv::addWeighted(matA, beta, matB, alpha, 0.0, resultMat);

    // --- 5. 转换并返回结果 ---
    return ImageConverter::matToQImage(resultMat);
}
