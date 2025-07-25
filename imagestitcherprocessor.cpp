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
// File: imagestitcherprocessor.cpp
//
// Description:
// ImageStitcherProcessor 类的实现文件。该文件包含了使用OpenCV的
// Stitcher类来实现图像拼接的逻辑。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "imagestitcherprocessor.h"
#include <opencv2/stitching.hpp>
#include <QDebug>

/**
 * @brief ImageStitcherProcessor 构造函数。
 *
 * 目前为空，因为OpenCV的Stitcher对象是在process方法中按需创建的。
 */
ImageStitcherProcessor::ImageStitcherProcessor() {}

/**
 * @brief 对外提供的处理接口，用于拼接一系列图像。
 *
 * 该方法使用OpenCV的Stitcher类来自动完成特征检测、匹配、变换矩阵估计
 * 和图像融合等一系列复杂的步骤。
 *
 * @param images 一个包含待拼接图像 (cv::Mat) 的向量。
 * @return 返回拼接后的全景图像 (cv::Mat)。如果拼接失败，则返回一个空的Mat。
 */
cv::Mat ImageStitcherProcessor::process(const std::vector<cv::Mat> &images)
{
    // --- 1. 输入验证 ---
    // 至少需要两张图片才能进行拼接
    if (images.size() < 2) {
        qWarning() << "需要至少两张图片才能拼接。";
        return cv::Mat();
    }

    // --- 2. 创建并配置Stitcher对象 ---
    cv::Mat pano; // 用于存储最终的全景图
    // 使用OpenCV的智能指针 cv::Ptr 来管理Stitcher对象
    // cv::Stitcher::create 创建一个默认配置的Stitcher实例
    // cv::Stitcher::PANORAMA 是默认模式，适用于相机水平移动拍摄的图像序列
    cv::Ptr<cv::Stitcher> stitcher = cv::Stitcher::create(cv::Stitcher::PANORAMA);

    // --- 3. 执行拼接 ---
    // stitcher->stitch 会执行整个拼接流程
    cv::Stitcher::Status status = stitcher->stitch(images, pano);

    // --- 4. 检查拼接结果 ---
    if (status != cv::Stitcher::OK) {
        // 如果状态不是OK，说明拼接失败
        // 常见失败原因包括：图像间没有足够的重叠区域、特征点无法匹配等
        qWarning() << "无法拼接，错误代码：" << int(status);
        return cv::Mat();
    }

    // --- 5. 返回结果 ---
    qDebug() << "拼接成功。";
    return pano;
}
