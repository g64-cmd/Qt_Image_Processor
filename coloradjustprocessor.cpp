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
// File: coloradjustprocessor.cpp
//
// Description:
// ColorAdjustProcessor 类的实现文件。该文件包含了色彩调整算法
// 的具体实现。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "coloradjustprocessor.h"
#include "imageconverter.h"
#include <opencv2/opencv.hpp>
#include <vector>

/**
 * @brief 调整图像的亮度和对比度。
 *
 * 使用OpenCV的 `convertTo` 方法高效地应用线性变换 `g(x) = α*f(x) + β`，
 * 其中 α 对应对比度，β 对应亮度。
 * @param sourceImage 原始 QImage 图像。
 * @param brightness 亮度调整值 (-100 to 100)。
 * @param contrast 对比度调整值 (-100 to 100)。
 * @return 调整后的 QImage。
 */
QImage ColorAdjustProcessor::adjustBrightnessContrast(const QImage &sourceImage, int brightness, int contrast)
{
    if (sourceImage.isNull()) {
        return sourceImage;
    }

    cv::Mat srcMat = ImageConverter::qImageToMat(sourceImage);
    if (srcMat.empty()) return sourceImage;

    // 将 contrast (-100 to 100) 映射到 alpha (0.0 to 2.0)
    double alpha = 1.0 + contrast / 100.0;
    // 将 brightness (-100 to 100) 直接用作 beta
    double beta = brightness;

    cv::Mat resultMat;
    // convertTo 执行像素级的操作：resultMat(i,j) = saturate_cast<uchar>(alpha * srcMat(i,j) + beta)
    srcMat.convertTo(resultMat, -1, alpha, beta);

    return ImageConverter::matToQImage(resultMat);
}

/**
 * @brief 调整图像的饱和度和色相。
 *
 * 该方法将图像转换到HSV色彩空间进行处理，因为HSV空间将色相(H)、
 * 饱和度(S)和明度(V)分离为独立的通道，便于单独调整。
 * @param sourceImage 原始 QImage 图像。
 * @param saturation 饱和度调整值 (-100 to 100)。
 * @param hue 色相偏移值 (-180 to 180)。
 * @return 调整后的 QImage。
 */
QImage ColorAdjustProcessor::adjustSaturationHue(const QImage &sourceImage, int saturation, int hue)
{
    if (sourceImage.isNull()) {
        return sourceImage;
    }

    cv::Mat srcMat = ImageConverter::qImageToMat(sourceImage);
    if (srcMat.empty()) return sourceImage;

    // --- 1. 转换到 HSV 色彩空间 ---
    cv::Mat hsvMat;
    cv::cvtColor(srcMat, hsvMat, cv::COLOR_BGR2HSV);

    // --- 2. 分离通道 ---
    std::vector<cv::Mat> hsvChannels;
    cv::split(hsvMat, hsvChannels);

    // --- 3. 调整饱和度 (Saturation) ---
    if (saturation != 0) {
        // 将 saturation (-100 to 100) 映射到增益因子 (0.0 to 2.0)
        double satGain = 1.0 + saturation / 100.0;

        // 为避免溢出，先将饱和度通道转换为浮点数类型进行乘法运算
        hsvChannels[1].convertTo(hsvChannels[1], CV_64F);
        hsvChannels[1] = hsvChannels[1] * satGain;

        // 将超出255的值截断为255
        cv::threshold(hsvChannels[1], hsvChannels[1], 255, 255, cv::THRESH_TRUNC);
        // 转换回8位无符号整数类型
        hsvChannels[1].convertTo(hsvChannels[1], CV_8U);
    }

    // --- 4. 调整色相 (Hue) ---
    if (hue != 0) {
        // 为避免在加法中溢出或出现负数，先转换为32位有符号整数
        hsvChannels[0].convertTo(hsvChannels[0], CV_32S);

        for(int i = 0; i < hsvChannels[0].rows; ++i) {
            for(int j = 0; j < hsvChannels[0].cols; ++j) {
                int &pixel = hsvChannels[0].at<int>(i,j);
                // 直接加上偏移量
                pixel = (pixel + hue) % 180;
                // 如果结果为负，则加上180使其回到0-179的范围内 (OpenCV中H通道范围)
                if(pixel < 0) {
                    pixel += 180;
                }
            }
        }
        // 转换回8位无符号整数类型
        hsvChannels[0].convertTo(hsvChannels[0], CV_8U);
    }

    // --- 5. 合并通道并转换回 BGR ---
    cv::merge(hsvChannels, hsvMat);
    cv::Mat resultMat;
    cv::cvtColor(hsvMat, resultMat, cv::COLOR_HSV2BGR);

    return ImageConverter::matToQImage(resultMat);
}
