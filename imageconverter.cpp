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
// File: imageconverter.cpp
//
// Description:
// ImageConverter 类的实现文件。该文件包含了在 QImage 和 cv::Mat
// 之间进行数据转换的具体实现。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "imageconverter.h"

/**
 * @brief 将 cv::Mat 转换为 QImage。
 *
 * 根据输入 cv::Mat 的类型（通道数），选择合适的 QImage 格式进行转换。
 * @param mat 输入的 OpenCV Mat 对象。
 * @return 转换后的 QImage 对象。如果格式不支持，则返回空的 QImage。
 */
QImage ImageConverter::matToQImage(const cv::Mat &mat)
{
    switch (mat.type()) {
    // Case 1: 8位单通道 (灰度图)
    case CV_8UC1: {
        // 使用mat的数据指针构造QImage。
        // mat.step是每行的字节数。
        // QImage::Format_Grayscale8 是对应的格式。
        QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8);
        // 返回一个深拷贝，确保QImage有自己的数据，而不是共享mat的内存。
        return image.copy();
    }

    // Case 2: 8位3通道 (BGR彩色图)
    case CV_8UC3: {
        // 使用mat的数据指针构造QImage，格式为RGB888。
        QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_RGB888);
        // OpenCV的默认颜色顺序是BGR，而Qt是RGB，所以需要交换R和B通道。
        return image.rgbSwapped();
    }

    // Case 3: 8位4通道 (BGRA彩色图)
    case CV_8UC4: {
        // 使用mat的数据指针构造QImage，格式为ARGB32。
        QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_ARGB32);
        // 返回一个深拷贝。
        return image.copy();
    }

    // Default: 不支持的格式
    default:
        break;
    }

    return QImage();
}

/**
 * @brief 将 QImage 转换为 cv::Mat。
 *
 * 根据输入 QImage 的格式，创建对应类型的 cv::Mat。
 * @param image 输入的 QImage 对象。
 * @return 转换后的 OpenCV Mat 对象。
 */
cv::Mat ImageConverter::qImageToMat(const QImage &image)
{
    cv::Mat mat;
    switch (image.format()) {
    // Case 1: 32位ARGB格式 (有Alpha通道)
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32: // 在Qt中，这个格式通常也包含一个未使用的alpha字节
    case QImage::Format_ARGB32_Premultiplied:
        // 创建一个指向QImage数据缓冲区的4通道Mat。
        // const_cast是必要的，因为cv::Mat构造函数需要一个非const指针，
        // 但我们通过最后的.clone()确保不会修改原始QImage数据。
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()), image.bytesPerLine());
        break;

    // Case 2: 24位RGB格式
    case QImage::Format_RGB888:
        // 创建一个指向QImage数据缓冲区的3通道Mat。
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, const_cast<uchar*>(image.bits()), image.bytesPerLine());
        // Qt的RGB888是RGB顺序，需要转换为OpenCV的BGR顺序。
        cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
        break;

    // Case 3: 8位灰度或索引图
    case QImage::Format_Grayscale8:
    case QImage::Format_Indexed8: // 索引图可以近似看作灰度图处理
        // 创建一个指向QImage数据缓冲区的单通道Mat。
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, const_cast<uchar*>(image.bits()), image.bytesPerLine());
        break;

    // Default: 其他不支持的格式
    default:
        // 将任何其他格式的图像先转换为一个标准的32位ARGB格式。
        QImage temp = image.convertToFormat(QImage::Format_ARGB32);
        // 然后按照ARGB格式进行处理。
        mat = cv::Mat(temp.height(), temp.width(), CV_8UC4, const_cast<uchar*>(temp.bits()), temp.bytesPerLine());
        break;
    }

    // 返回一个克隆。这非常重要，因为它创建了一个独立的数据副本，
    // 使返回的cv::Mat的生命周期与原始QImage无关。
    return mat.clone();
}
