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

#ifndef IMAGESTITCHERPROCESSOR_H
#define IMAGESTITCHERPROCESSOR_H

// =============================================================================
// File: imagestitcherprocessor.h
//
// Description:
// 该文件定义了 ImageStitcherProcessor 类，这是一个负责执行
// 图像拼接（全景）算法的处理器。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <opencv2/opencv.hpp>
#include <vector>

/**
 * @class ImageStitcherProcessor
 * @brief 图像拼接功能处理器。
 *
 * 封装了使用OpenCV的Stitcher类来将多张图像拼接成一张全景图的逻辑。
 * 注意：这是一个有状态的类，需要被实例化后使用。
 */
class ImageStitcherProcessor
{
public:
    /**
     * @brief 构造函数。
     */
    ImageStitcherProcessor();

    /**
     * @brief 对外提供的处理接口，用于拼接一系列图像。
     * @param images 一个包含待拼接图像 (cv::Mat) 的向量。
     * @return 返回拼接后的全景图像 (cv::Mat)。如果拼接失败，则返回一个空的Mat。
     */
    cv::Mat process(const std::vector<cv::Mat> &images);
};

#endif // IMAGESTITCHERPROCESSOR_H
