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
// File: imagetexturetransferprocessor.cpp
//
// Description:
// ImageTextureTransferProcessor 类的实现文件。该文件实现了基于Efros和Freeman
// 的 "Image Quilting" 思想的纹理迁移算法，并结合了图像金字塔、
// 显著性检测和颜色统计匹配等多种技术。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "imagetexturetransferprocessor.h"
#include "imageconverter.h"
#include <opencv2/opencv.hpp>
#include <opencv2/saliency.hpp>
#include <vector>
#include <limits>
#include <QDebug>
#include <algorithm>

// =============================================================================
// 静态辅助函数 (Static Helper Functions)
// 这些函数仅在此文件内部使用，用于支持主处理流程。
// =============================================================================

/**
 * @brief 使用动态规划计算两个重叠区域之间的最小误差边界。
 *
 * 这个函数实现了 "Image Quilting" 中的核心步骤之一：在拼接图像块时，
 * 找到一条使两个块在重叠区域的差异最小的路径（边界）。
 *
 * @param overlap_new 新图像块的重叠区域。
 * @param overlap_old 已合成图像的重叠区域。
 * @param is_vertical 如果为true，则寻找垂直边界；否则寻找水平边界。
 * @return 返回一个二值掩码 (CV_8UC1)，其中值为255的区域表示应从新图像块中拾取。
 */
static cv::Mat calculateMinErrorCut(const cv::Mat& overlap_new, const cv::Mat& overlap_old, bool is_vertical) {
    if (overlap_new.empty() || overlap_old.empty() || overlap_new.size() != overlap_old.size()) {
        // 如果输入无效，返回一个完全接受新块的掩码
        return cv::Mat(overlap_new.size(), CV_8UC1, cv::Scalar(255));
    }

    // 1. 计算误差矩阵：两个重叠区域的像素差的绝对值
    cv::Mat err;
    cv::absdiff(overlap_new, overlap_old, err);
    // 将BGR误差转换为单通道的亮度误差
    cv::transform(err, err, cv::Matx13f(0.299f, 0.587f, 0.114f));
    err.convertTo(err, CV_32F);

    // 对于水平边界，将矩阵转置，当作垂直边界问题处理
    if (!is_vertical) {
        cv::transpose(err, err);
    }

    // 2. 动态规划：计算累积误差矩阵 M
    cv::Mat M = cv::Mat::zeros(err.size(), CV_32F);
    err.row(0).copyTo(M.row(0)); // 第一行累积误差就是其自身误差

    for (int i = 1; i < err.rows; ++i) {
        for (int j = 0; j < err.cols; ++j) {
            // 当前点的累积误差 = 当前点的像素误差 + 上方三个邻居的最小累积误差
            float min_prev = M.at<float>(i - 1, j);
            if (j > 0) min_prev = std::min(min_prev, M.at<float>(i - 1, j - 1));
            if (j < err.cols - 1) min_prev = std::min(min_prev, M.at<float>(i - 1, j + 1));
            M.at<float>(i, j) = err.at<float>(i, j) + min_prev;
        }
    }

    // 3. 回溯：从累积误差矩阵 M 中找到最小误差路径
    cv::Mat mask = cv::Mat::zeros(err.size(), CV_8UC1);
    double min_val;
    cv::Point min_loc;
    // 在最后一行找到最小累积误差的位置，作为路径的终点
    cv::minMaxLoc(M.row(err.rows - 1), &min_val, nullptr, &min_loc, nullptr);

    int current_j = min_loc.x;
    for (int i = err.rows - 1; i >= 0; --i) {
        // 将路径右侧的像素标记为255（接受新块）
        if (current_j >= 0 && current_j < mask.cols) {
            mask(cv::Rect(current_j, i, mask.cols - current_j, 1)).setTo(255);
        }

        // 寻找上一行中导致当前最小路径的邻居
        if (i > 0) {
            int prev_j = current_j;
            float up_val = M.at<float>(i - 1, prev_j);
            float left_val = (prev_j > 0) ? M.at<float>(i - 1, prev_j - 1) : std::numeric_limits<float>::max();
            float right_val = (prev_j < err.cols - 1) ? M.at<float>(i - 1, prev_j + 1) : std::numeric_limits<float>::max();

            if (left_val <= up_val && left_val <= right_val) {
                current_j = prev_j - 1;
            } else if (right_val < up_val && right_val < left_val) {
                current_j = prev_j + 1;
            }
        }
    }

    // 如果是水平边界，将掩码转置回来
    if (!is_vertical) {
        cv::transpose(mask, mask);
    }
    return mask;
}

/**
 * @brief 使用显著性引导采样和组合误差函数，从源纹理中找到最佳匹配块。
 * @return 返回从源纹理中裁剪出的最佳匹配图像块 (Lab格式)。
 */
static cv::Mat findBestMatch(const cv::Mat& target_patch_lab, const cv::Mat& overlap_mask, const cv::Mat& synthesized_region_lab,
                             const cv::Mat& source_lab, const cv::Mat& source_grad_l, const cv::Mat& source_saliency,
                             double alpha, double beta, int patch_size)
{
    // 使用线程局部变量的随机数生成器，以支持多线程
    static thread_local cv::RNG rng(cv::getTickCount());

    int y_range = source_lab.rows - patch_size;
    int x_range = source_lab.cols - patch_size;
    if (y_range < 0 || x_range < 0) return cv::Mat();

    // 预计算目标块的梯度
    std::vector<cv::Mat> target_channels;
    cv::split(target_patch_lab, target_channels);
    cv::Mat target_grad_l;
    cv::Sobel(target_channels[0], target_grad_l, CV_32F, 1, 1);

    // 1. 候选块采样
    int num_candidates = 500;
    std::vector<cv::Rect> candidate_rects;
    std::vector<double> candidate_errors;

    // 准备显著性图用于引导采样
    cv::Mat saliency_prob;
    cv::normalize(source_saliency, saliency_prob, 1.0, 0.0, cv::NORM_MINMAX, CV_32F);
    saliency_prob += 0.1; // 避免概率为0

    for (int i = 0; i < num_candidates; ++i) {
        // 混合采样策略：大部分随机采样，小部分由显著性图引导
        cv::Point sample_point;
        cv::minMaxLoc(saliency_prob, nullptr, nullptr, nullptr, &sample_point);
        int y = rng.uniform(0, y_range);
        int x = rng.uniform(0, x_range);
        if (rng.uniform(0, 10) > 5) { // 有一定概率在最显著点附近采样
            y = std::min(y_range - 1, std::max(0, sample_point.y - patch_size / 2));
            x = std::min(x_range - 1, std::max(0, sample_point.x - patch_size / 2));
        }

        cv::Rect source_rect(x, y, patch_size, patch_size);
        cv::Rect crop_rect(0, 0, target_patch_lab.cols, target_patch_lab.rows);

        // 2. 计算误差
        // a. 边界误差 (Boundary Error): 候选块与已合成区域在重叠部分的SSD(Sum of Squared Differences)
        double boundary_error = 0;
        if (cv::countNonZero(overlap_mask) > 0) {
            cv::Mat source_patch_lab_cropped = source_lab(source_rect)(crop_rect);
            cv::Mat diff_boundary;
            cv::absdiff(source_patch_lab_cropped, synthesized_region_lab, diff_boundary);
            cv::multiply(diff_boundary, diff_boundary, diff_boundary); // 平方
            std::vector<cv::Mat> diff_channels;
            cv::split(diff_boundary, diff_channels);
            // 应用重叠掩码
            diff_channels[0] = diff_channels[0].mul(overlap_mask, 1.0 / 255.0);
            diff_channels[1] = diff_channels[1].mul(overlap_mask, 1.0 / 255.0);
            diff_channels[2] = diff_channels[2].mul(overlap_mask, 1.0 / 255.0);
            cv::Scalar ssd_boundary = cv::sum(diff_channels[0]) + cv::sum(diff_channels[1]) + cv::sum(diff_channels[2]);
            boundary_error = ssd_boundary[0] / (cv::countNonZero(overlap_mask) + 1e-6);
        }

        // b. 内容误差 (Content Error): 候选块与目标内容块在颜色和梯度上的差异
        cv::Mat source_patch_lab_cropped = source_lab(source_rect)(crop_rect);
        std::vector<cv::Mat> source_channels;
        cv::split(source_patch_lab_cropped, source_channels);
        cv::Mat diff_lum;
        cv::absdiff(source_channels[0], target_channels[0], diff_lum); // 亮度差异
        diff_lum.convertTo(diff_lum, CV_32F);
        cv::multiply(diff_lum, diff_lum, diff_lum);
        double lum_error = cv::mean(diff_lum)[0];

        cv::Mat source_grad_l_cropped = source_grad_l(source_rect)(crop_rect);
        cv::Mat diff_grad;
        cv::absdiff(source_grad_l_cropped, target_grad_l, diff_grad); // 梯度差异
        double grad_error = cv::mean(diff_grad)[0];

        // 组合内容误差
        double content_error = (1.0 - beta) * lum_error + beta * grad_error;

        // c. 总误差 (Total Error): 边界误差和内容误差的加权和
        double total_error = alpha * boundary_error + (1.0 - alpha) * content_error;

        candidate_rects.push_back(source_rect);
        candidate_errors.push_back(total_error);
    }

    // 3. 选择最佳块
    if (candidate_errors.empty()) return cv::Mat();
    double min_error = *std::min_element(candidate_errors.begin(), candidate_errors.end());

    // 为了增加随机性，不总是选择误差最小的，而是在一个容忍度范围内随机选择一个
    std::vector<cv::Rect> tolerated_candidates;
    double tolerance = 1.2;
    for (size_t i = 0; i < candidate_errors.size(); ++i) {
        if (candidate_errors[i] <= min_error * tolerance) {
            tolerated_candidates.push_back(candidate_rects[i]);
        }
    }

    if (tolerated_candidates.empty()) { // 如果没有在容忍度内的，就选最小的那个
        auto min_it = std::min_element(candidate_errors.begin(), candidate_errors.end());
        size_t min_idx = std::distance(candidate_errors.begin(), min_it);
        return source_lab(candidate_rects[min_idx]).clone();
    }

    // 从容忍度范围内的候选中随机选择一个
    int random_idx = rng.uniform(0, (int)tolerated_candidates.size());
    return source_lab(tolerated_candidates[random_idx]).clone();
}

/**
 * @brief 对外提供的唯一处理接口，执行纹理迁移。
 * @param contentImage 内容图片 (QImage)。
 * @param textureImage 纹理/风格图片 (QImage)。
 * @return 迁移了纹理的新 QImage。
 */
QImage ImageTextureTransferProcessor::process(const QImage &contentImage, const QImage &textureImage)
{
    try {
        if (contentImage.isNull() || textureImage.isNull()) return QImage();

        // --- 1. 预处理：构建图像金字塔和辅助数据 ---
        cv::Mat content_mat = ImageConverter::qImageToMat(contentImage);
        cv::Mat texture_mat = ImageConverter::qImageToMat(textureImage);

        int num_levels = 4; // 金字塔层数
        std::vector<cv::Mat> content_pyramid_lab, texture_pyramid_lab;
        cv::Mat content_lab, texture_lab;

        // 转换到Lab颜色空间，因为Lab空间中亮度和颜色是分离的
        cv::cvtColor(content_mat, content_lab, cv::COLOR_BGR2Lab);
        cv::cvtColor(texture_mat, texture_lab, cv::COLOR_BGR2Lab);

        // 构建内容图和纹理图的金字塔
        cv::buildPyramid(content_lab, content_pyramid_lab, num_levels);
        cv::buildPyramid(texture_lab, texture_pyramid_lab, num_levels);

        // 为纹理金字塔的每一层预计算梯度图和显著性图
        std::vector<cv::Mat> texture_pyramid_grad_l;
        std::vector<cv::Mat> texture_pyramid_saliency;
        cv::Ptr<cv::saliency::Saliency> saliency = cv::saliency::StaticSaliencySpectralResidual::create();

        for (const auto& tex_level_lab : texture_pyramid_lab) {
            std::vector<cv::Mat> tex_channels;
            cv::split(tex_level_lab, tex_channels);
            cv::Mat grad_l;
            cv::Sobel(tex_channels[0], grad_l, CV_32F, 1, 1);
            texture_pyramid_grad_l.push_back(grad_l);

            cv::Mat saliency_map;
            cv::Mat tex_level_bgr;
            cv::cvtColor(tex_level_lab, tex_level_bgr, cv::COLOR_Lab2BGR);
            saliency->computeSaliency(tex_level_bgr, saliency_map);
            texture_pyramid_saliency.push_back(saliency_map);
        }

        // --- 2. 从粗到细，逐层合成 ---
        cv::Mat result_lab;
        for (int level = num_levels; level >= 0; --level) {
            qDebug() << "--- Processing Pyramid Level" << level << "---";
            const cv::Mat& current_content_lab = content_pyramid_lab[level];
            const cv::Mat& current_texture_lab = texture_pyramid_lab[level];
            const cv::Mat& current_texture_grad_l = texture_pyramid_grad_l[level];
            const cv::Mat& current_texture_saliency = texture_pyramid_saliency[level];

            if (level == num_levels) { // 最粗糙的一层，从零开始合成
                result_lab = cv::Mat::zeros(current_content_lab.size(), current_content_lab.type());
            } else { // 将上一层的合成结果放大，作为当前层的起点
                cv::pyrUp(result_lab, result_lab, current_content_lab.size());
            }

            // --- 3. 基于块的合成 (Patch-based Synthesis) ---
            int patch_size = std::max(5, std::min(current_content_lab.rows, current_content_lab.cols) / 8);
            if (patch_size % 2 == 0) patch_size++; // 确保patch大小为奇数
            if (patch_size >= current_texture_lab.rows || patch_size >= current_texture_lab.cols) continue;
            int overlap = std::max(1, patch_size / 6);

            // alpha控制边界误差和内容误差的权重，在粗糙层更注重内容，在精细层更注重边界平滑
            double alpha = 0.1 + 0.8 * (level / (double)num_levels);
            double beta = 0.7; // beta控制亮度误差和梯度误差的权重

            // 逐块扫描并合成图像
            for (int y = 0; y < current_content_lab.rows; y += patch_size - overlap) {
                for (int x = 0; x < current_content_lab.cols; x += patch_size - overlap) {
                    int w = std::min(patch_size, current_content_lab.cols - x);
                    int h = std::min(patch_size, current_content_lab.rows - y);
                    if (w <= overlap || h <= overlap) continue;

                    cv::Rect current_rect(x, y, w, h);
                    cv::Mat target_patch = current_content_lab(current_rect);
                    cv::Mat synthesized_region = result_lab(current_rect);

                    // 创建重叠区域的掩码
                    cv::Mat overlap_mask = cv::Mat::zeros(h, w, CV_8UC1);
                    if (x > 0) overlap_mask(cv::Rect(0, 0, std::min(overlap, w), h)).setTo(255);
                    if (y > 0) overlap_mask(cv::Rect(0, 0, w, std::min(overlap, h))).setTo(255);

                    // 找到最佳匹配块
                    cv::Mat best_match_full = findBestMatch(target_patch, overlap_mask, synthesized_region, current_texture_lab, current_texture_grad_l, current_texture_saliency, alpha, beta, patch_size);
                    if (best_match_full.empty()) continue;
                    cv::Mat best_match = best_match_full(cv::Rect(0, 0, w, h));

                    // --- 4. 最小误差边界切割与融合 ---
                    cv::Mat final_mask = cv::Mat(h, w, CV_8UC1, cv::Scalar(255));
                    if (x > 0) { // 处理左侧重叠
                        cv::Rect rect(0, 0, std::min(overlap, w), h);
                        cv::Mat cut_mask = calculateMinErrorCut(best_match(rect), synthesized_region(rect), true);
                        cut_mask.copyTo(final_mask(rect));
                    }
                    if (y > 0) { // 处理上方重叠
                        cv::Rect rect(0, 0, w, std::min(overlap, h));
                        cv::Mat cut_mask = calculateMinErrorCut(best_match(rect), synthesized_region(rect), false);
                        cv::bitwise_and(final_mask(rect), cut_mask, final_mask(rect));
                    }

                    // 将最佳块通过计算出的边界掩码拼接到结果图上
                    best_match.copyTo(result_lab(current_rect), final_mask);
                }
            }
        }

        // --- 5. 颜色保持 ---
        // 为了保留原始内容的颜色，只使用合成结果的L通道（亮度/纹理），
        // 而使用原始内容的a和b通道（颜色）。
        qDebug() << "Performing final color preservation...";
        std::vector<cv::Mat> result_channels, original_content_channels;
        cv::split(result_lab, result_channels);
        cv::split(content_lab, original_content_channels);
        std::vector<cv::Mat> final_channels = { result_channels[0], original_content_channels[1], original_content_channels[2] };
        cv::merge(final_channels, result_lab);
        qDebug() << "Color preservation complete.";

        // --- 6. 转换回BGR并返回 ---
        cv::Mat result_bgr;
        cv::cvtColor(result_lab, result_bgr, cv::COLOR_Lab2BGR);
        return ImageConverter::matToQImage(result_bgr);

    } catch (const cv::Exception& e) {
        // 捕获并报告任何OpenCV异常
        qDebug() << "!!!!!!!!!! An OpenCV Exception was caught in Texture Transfer !!!!!!!!!!";
        qDebug() << "Error Message:" << e.msg.c_str();
        return QImage();
    }
}
