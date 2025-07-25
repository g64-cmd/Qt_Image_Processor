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

#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H

// =============================================================================
// File: imageconverter.h
//
// Description:
// 该文件定义了 ImageConverter 类，这是一个静态工具类，
// 提供了在 Qt 的 QImage 和 OpenCV 的 cv::Mat 之间进行相互转换的功能。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QImage>
#include <opencv2/opencv.hpp>

/**
 * @class ImageConverter
 * @brief 图像格式转换辅助类。
 *
 * 提供静态方法用于在 Qt 的 QImage 和 OpenCV 的 cv::Mat 之间进行转换。
 * 这是连接 Qt GUI 和 OpenCV 图像处理核心的重要桥梁。
 */
class ImageConverter
{
public:
    /**
     * @brief 删除默认构造函数，以防止该类的实例化。
     */
    ImageConverter() = delete;

    /**
     * @brief 将 cv::Mat 转换为 QImage。
     *
     * 处理单通道（灰度）、3通道（BGR）和4通道（BGRA）的Mat。
     * @param mat 输入的 OpenCV Mat 对象 (必须是 CV_8UC1, CV_8UC3 或 CV_8UC4 类型)。
     * @return 转换后的 QImage 对象。
     */
    static QImage matToQImage(const cv::Mat &mat);

    /**
     * @brief 将 QImage 转换为 cv::Mat。
     *
     * 处理多种QImage格式，并将其统一转换为主流的OpenCV格式。
     * @param image 输入的 QImage 对象。
     * @return 转换后的 OpenCV Mat 对象。
     */
    static cv::Mat qImageToMat(const QImage &image);
};

#endif // IMAGECONVERTER_H
