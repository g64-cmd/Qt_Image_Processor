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
// File: beautyprocessor.cpp
//
// Description:
// BeautyProcessor 类的实现文件。该文件包含了美颜算法的具体实现，
// 包括模型加载、人脸检测、关键点定位，以及磨皮和瘦脸的图像处理逻辑。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "beautyprocessor.h"
#include "imageconverter.h" // 包含QImage和cv::Mat之间的转换工具

#include <dlib/opencv.h>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <vector>

/**
 * @brief 从Qt资源文件中提取数据并保存到临时文件中。
 *
 * dlib的模型加载函数需要一个磁盘文件路径，但我们的模型文件打包在.qrc资源中。
 * 此辅助函数解决了这个问题，它将资源数据读取出来，写入一个临时的磁盘文件，
 * 并返回该临时文件的智能指针，该指针会在其作用域结束时自动删除文件。
 *
 * @param resourcePath Qt资源路径 (e.g., ":/models/model.dat")。
 * @return 指向QTemporaryFile的unique_ptr，如果失败则返回nullptr。
 */
static std::unique_ptr<QTemporaryFile> extractResource(const QString& resourcePath)
{
    QFile resourceFile(resourcePath);
    if (!resourceFile.exists()) {
        qWarning() << "Resource not found:" << resourcePath;
        return nullptr;
    }

    // 创建一个唯一的临时文件
    auto tempFile = std::make_unique<QTemporaryFile>();
    if (tempFile->open()) {
        if (resourceFile.open(QIODevice::ReadOnly)) {
            // 将资源内容写入临时文件
            tempFile->write(resourceFile.readAll());
            resourceFile.close();
            tempFile->close(); // 必须关闭文件才能让dlib从路径中读取
            return tempFile;
        }
    }
    qWarning() << "Failed to open or write to temporary file for resource:" << resourcePath;
    return nullptr;
}

/**
 * @brief BeautyProcessor 构造函数。
 *
 * 初始化dlib的人脸检测器，并从资源文件中加载面部关键点预测模型。
 */
BeautyProcessor::BeautyProcessor()
{
    // 获取dlib内置的正面人脸检测器
    face_detector = dlib::get_frontal_face_detector();

    // 加载关键点预测模型
    QString landmarkModelResourcePath = ":/resources/models/shape_predictor_68_face_landmarks.dat";
    tempModelFile = extractResource(landmarkModelResourcePath);

    if (tempModelFile) {
        try {
            // 从临时文件路径反序列化（加载）模型
            dlib::deserialize(tempModelFile->fileName().toStdString()) >> landmark_predictor;
            qDebug() << "Successfully loaded landmark model from resource.";
        } catch (const dlib::serialization_error& e) {
            // 如果模型文件损坏或不兼容，则捕获异常
            qWarning() << "Error: Could not load landmark model from resource" << landmarkModelResourcePath << e.what();
        }
    }
}

/**
 * @brief 对源图像执行美颜处理。
 *
 * 这是该类的主要公共接口。它按顺序执行人脸检测、关键点定位、
 * 瘦脸和磨皮操作。
 * @param sourceImage 待处理的原始 QImage 图像。
 * @param smoothLevel 磨皮等级 (0-100)。
 * @param thinLevel 瘦脸等级 (0-100)。
 * @return 返回处理后的 QImage 图像。如果未检测到人脸或初始化失败，则返回原始图像。
 */
QImage BeautyProcessor::process(const QImage &sourceImage, int smoothLevel, int thinLevel)
{
    // --- 1. 有效性检查 ---
    if (sourceImage.isNull() || landmark_predictor.num_parts() == 0) {
        qWarning() << "Beauty processor not initialized or models failed to load, skipping.";
        return sourceImage;
    }

    // --- 2. 格式转换 ---
    cv::Mat originalMat = ImageConverter::qImageToMat(sourceImage);
    if (originalMat.empty()) return sourceImage;
    // 确保图像是3通道BGR格式，dlib需要这种格式
    if (originalMat.channels() == 4) {
        cv::cvtColor(originalMat, originalMat, cv::COLOR_BGRA2BGR);
    }

    // --- 3. 图像预处理与人脸检测 ---
    // 将cv::Mat封装为dlib可以处理的图像类型
    dlib::cv_image<dlib::bgr_pixel> dlib_img(originalMat);
    dlib::cv_image<dlib::bgr_pixel> detection_img = dlib_img;

    // 对于尺寸过小的图片，先放大再进行检测可以提高准确率
    float scale = 1.0f;
    const int min_size_for_detection = 250;
    if (originalMat.cols < min_size_for_detection || originalMat.rows < min_size_for_detection) {
        scale = std::max(2.0f, min_size_for_detection / static_cast<float>(std::min(originalMat.cols, originalMat.rows)));
        cv::Mat upscaled_mat;
        cv::resize(originalMat, upscaled_mat, cv::Size(), scale, scale, cv::INTER_CUBIC);
        detection_img = dlib::cv_image<dlib::bgr_pixel>(upscaled_mat);
        qDebug() << "Image is small, upscaling by" << scale << "for detection.";
    }

    std::vector<dlib::rectangle> faces = face_detector(detection_img);
    qDebug() << "Detected" << faces.size() << "face(s).";

    if (faces.empty()) {
        return sourceImage; // 未检测到人脸，返回原图
    }

    cv::Mat processedMat = originalMat.clone();

    // --- 4. 迭代处理每个检测到的人脸 ---
    for (const auto& face_upscaled : faces) {
        // 如果图像被放大了，需要将检测到的脸部矩形缩放回原始尺寸
        dlib::rectangle face;
        if (scale > 1.0f) {
            face = dlib::rectangle(
                (long)(face_upscaled.left() / scale),
                (long)(face_upscaled.top() / scale),
                (long)(face_upscaled.right() / scale),
                (long)(face_upscaled.bottom() / scale)
                );
        } else {
            face = face_upscaled;
        }

        // 在原始尺寸的图像上定位面部关键点
        dlib::full_object_detection landmarks = landmark_predictor(dlib_img, face);

        // --- 5. 应用美颜效果 ---
        // 先应用瘦脸（结构变形），再应用磨皮（纹理处理）
        if (thinLevel > 0) {
            qDebug() << "--- Applying face thinning with level:" << thinLevel;
            applyFaceThinning(processedMat, landmarks, thinLevel);
        }
        if (smoothLevel > 0) {
            qDebug() << "--- Applying skin smoothing with level:" << smoothLevel;
            applySkinSmoothing(processedMat, landmarks, smoothLevel);
        }
    }

    // --- 6. 返回结果 ---
    return ImageConverter::matToQImage(processedMat);
}

/**
 * @brief 应用皮肤平滑（磨皮）效果。
 *
 * 使用高低频分离和双边滤波技术，在平滑皮肤的同时保留边缘细节。
 * @param image [in, out] 要处理的 cv::Mat 图像，效果将直接应用在此图像上。
 * @param landmarks 检测到的68个面部关键点。
 * @param level 磨皮强度 (0-100)。
 */
void BeautyProcessor::applySkinSmoothing(cv::Mat &image, const dlib::full_object_detection& landmarks, int level)
{
    if (level <= 0 || landmarks.num_parts() != 68) return;

    // 1. 创建皮肤区域的掩码(mask)
    // a. 使用面部外轮廓(0-16)和下巴(17-26)关键点创建一个凸包，作为初始皮肤区域
    std::vector<cv::Point> face_hull;
    for (unsigned long i = 0; i <= 16; ++i) face_hull.push_back(cv::Point(landmarks.part(i).x(), landmarks.part(i).y()));
    for (unsigned long i = 26; i >= 17; --i) face_hull.push_back(cv::Point(landmarks.part(i).x(), landmarks.part(i).y()));

    cv::Mat skin_mask = cv::Mat::zeros(image.size(), CV_8UC1);
    cv::fillConvexPoly(skin_mask, face_hull, 255);

    // b. 从掩码中排除眼睛、眉毛和嘴巴区域，因为这些区域不需要磨皮
    std::vector<cv::Point> left_eye, right_eye, mouth;
    for (unsigned long i = 36; i <= 41; ++i) left_eye.push_back(cv::Point(landmarks.part(i).x(), landmarks.part(i).y()));
    for (unsigned long i = 42; i <= 47; ++i) right_eye.push_back(cv::Point(landmarks.part(i).x(), landmarks.part(i).y()));
    for (unsigned long i = 48; i <= 59; ++i) mouth.push_back(cv::Point(landmarks.part(i).x(), landmarks.part(i).y()));
    cv::fillConvexPoly(skin_mask, left_eye, 0);
    cv::fillConvexPoly(skin_mask, right_eye, 0);
    cv::fillConvexPoly(skin_mask, mouth, 0);

    // c. 轻微模糊掩码边缘，使最终效果过渡更自然
    cv::GaussianBlur(skin_mask, skin_mask, cv::Size(15, 15), 0, 0);

    // 2. 应用高级表面模糊算法
    qDebug() << "Applying advanced surface blur with level:" << level;
    // a. 高斯模糊得到低频分量（模糊的背景）
    int kernel_size = (level / 10) * 2 + 1; // 磨皮等级越高，模糊半径越大
    cv::Mat low_freq;
    cv::GaussianBlur(image, low_freq, cv::Size(kernel_size, kernel_size), 0, 0);

    // b. 原图减去低频得到高频分量（细节、纹理）
    cv::Mat high_freq = image - low_freq;

    // c. 对低频分量使用双边滤波，这是一种保边滤波器，可以在平滑颜色的同时保留边缘
    cv::Mat enhanced_low_freq;
    int d = level / 10 + 5; // 双边滤波的邻域直径
    cv::bilateralFilter(low_freq, enhanced_low_freq, d, 150, 150);

    // d. 将处理过的低频和原始高频重新组合
    cv::Mat result = enhanced_low_freq + high_freq;

    // 3. 使用掩码将处理结果合成回原图
    result.copyTo(image, skin_mask);
}

/**
 * @brief 应用瘦脸效果。
 *
 * 通过计算像素位移并使用cv::remap实现液化效果。
 * @param image [in, out] 要处理的 cv::Mat 图像，效果将直接应用在此图像上。
 * @param landmarks 检测到的68个面部关键点。
 * @param level 瘦脸强度 (0-100)。
 */
void BeautyProcessor::applyFaceThinning(cv::Mat &image, const dlib::full_object_detection& landmarks, int level)
{
    if (level <= 0 || landmarks.num_parts() != 68) return;

    cv::Mat tempImage = image.clone();
    cv::Mat map_x = cv::Mat(image.size(), CV_32FC1);
    cv::Mat map_y = cv::Mat(image.size(), CV_32FC1);

    // 1. 定义关键点和参数
    dlib::point p_left_jaw = landmarks.part(3);
    dlib::point p_right_jaw = landmarks.part(13);
    dlib::point p_chin = landmarks.part(8);
    dlib::point p_nose_bridge = landmarks.part(27);

    // 2. 定义脸部中轴线和变形参数
    // 将dlib点转换为OpenCV浮点数点，以进行精确的向量计算
    cv::Point2f chin_pt(p_chin.x(), p_chin.y());
    cv::Point2f nose_pt(p_nose_bridge.x(), p_nose_bridge.y());

    // 计算从鼻子到下巴的中轴线向量
    cv::Point2f central_axis_vec = chin_pt - nose_pt;

    // 定义变形效果的影响半径，大致为鼻子到下巴的距离
    double radius = dlib::length(p_chin - landmarks.part(27));

    // 将UI的level (0-100) 映射到一个合适的变形强度
    double strength = level / 100.0 * 0.15;

    // 3. 生成重映射查找表 (remap maps)
    for (int y = 0; y < tempImage.rows; y++) {
        for (int x = 0; x < tempImage.cols; x++) {
            // 默认情况下，像素不移动
            map_x.at<float>(y, x) = static_cast<float>(x);
            map_y.at<float>(y, x) = static_cast<float>(y);

            // 判断当前像素在左脸还是右脸，并获取对应的脸颊关键点
            dlib::point jaw_pt = (x < nose_pt.x) ? p_left_jaw : p_right_jaw;

            double dist_to_jaw = dlib::length(dlib::point(x, y) - jaw_pt);

            // 只处理脸颊附近半径内的点
            if (dist_to_jaw < radius) {
                cv::Point2f current_pt_f(x, y);

                // 计算从鼻子到当前点的向量
                cv::Point2f vec_to_pt = current_pt_f - nose_pt;

                // 将 vec_to_pt 投影到中轴线 central_axis_vec 上
                float proj = vec_to_pt.dot(central_axis_vec) / (central_axis_vec.x*central_axis_vec.x + central_axis_vec.y*central_axis_vec.y);

                // 找到中轴线上与当前点最近的点
                cv::Point2f pt_on_axis = nose_pt + proj * central_axis_vec;

                // 计算从投影点到当前点的向量 (dx, dy)，这个向量大致垂直于中轴线
                float dx = x - pt_on_axis.x;
                float dy = y - pt_on_axis.y;

                // 计算位移缩放因子。离脸颊越远，效果越弱
                float scale = strength * std::pow(1.0 - dist_to_jaw / radius, 2);

                // 根据原始算法，计算新的像素坐标
                map_x.at<float>(y, x) = static_cast<float>(x + dx * scale);
                map_y.at<float>(y, x) = static_cast<float>(y + dy * scale);
            }
        }
    }

    // 4. 应用重映射
    cv::remap(tempImage, image, map_x, map_y, cv::INTER_LINEAR, cv::BORDER_REPLICATE);
}
