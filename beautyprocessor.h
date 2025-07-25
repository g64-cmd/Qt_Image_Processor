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

#ifndef BEAUTYPROCESSOR_H
#define BEAUTYPROCESSOR_H

// =============================================================================
// File: beautyprocessor.h
//
// Description:
// 该文件定义了 BeautyProcessor 类，负责执行实际的美颜算法。
// 它利用 dlib 库进行人脸检测和面部关键点定位，并结合 OpenCV
// 实现磨皮和瘦脸等图像处理效果。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QImage>
#include <QTemporaryFile>
#include <memory>

// --- 第三方库包含 ---
#include <dlib/image_io.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <opencv2/opencv.hpp>

/**
 * @class BeautyProcessor
 * @brief 封装了美颜处理算法的核心逻辑。
 *
 * 该类初始化所需的人脸检测和特征点预测模型，并提供一个统一的
 * process 接口，用于对输入的图像应用磨皮和瘦脸效果。
 */
class BeautyProcessor
{
public:
    /**
     * @brief 构造函数。
     *
     * 负责初始化dlib的人脸检测器和特征点预测器。
     * 特别地，它会从Qt资源文件中加载预训练的shape_predictor模型。
     */
    BeautyProcessor();

    /**
     * @brief 对源图像执行美颜处理。
     * @param sourceImage 待处理的原始 QImage 图像。
     * @param smoothLevel 磨皮等级 (0-100)。
     * @param thinLevel 瘦脸等级 (0-100)。
     * @return 返回处理后的 QImage 图像。如果未检测到人脸，则返回原始图像。
     */
    QImage process(const QImage &sourceImage, int smoothLevel, int thinLevel);

private:
    /**
     * @brief 应用磨皮效果。
     * @param image [in, out] 要处理的 cv::Mat 图像。
     * @param landmarks 检测到的面部关键点。
     * @param level 磨皮强度。
     */
    void applySkinSmoothing(cv::Mat &image, const dlib::full_object_detection& landmarks, int level);

    /**
     * @brief 应用瘦脸效果。
     * @param image [in, out] 要处理的 cv::Mat 图像。
     * @param landmarks 检测到的面部关键点。
     * @param level 瘦脸强度。
     */
    void applyFaceThinning(cv::Mat &image, const dlib::full_object_detection& landmarks, int level);

    // --- 成员变量 ---

    // dlib的人脸检测器，用于在图像中定位人脸。
    dlib::frontal_face_detector face_detector;

    // dlib的形状预测器，用于定位面部的68个关键点。
    dlib::shape_predictor landmark_predictor;

    // 使用智能指针管理一个临时文件。
    // dlib的模型加载器需要一个文件路径，因此我们将模型从Qt资源(.qrc)
    // 提取到一个临时文件中，然后加载它。
    // unique_ptr 会在 BeautyProcessor 对象销毁时自动删除该临时文件。
    std::unique_ptr<QTemporaryFile> tempModelFile;
};

#endif // BEAUTYPROCESSOR_H
