#include "imagetexturetransferprocessor.h"
#include "imageconverter.h" // 假设此文件提供了 QImage 和 cv::Mat 的转换
#include <opencv2/opencv.hpp>
#include <opencv2/saliency.hpp> // For Saliency
#include <vector>
#include <limits>
#include <QDebug> // 添加 qDebug 头文件
#include <algorithm>

// --- 新算法的辅助函数 ---

/**
 * @brief Calculates the minimum error boundary cut between two overlapping patches.
 * @param overlap_new The new patch's overlap region.
 * @param overlap_old The existing synthesized region's overlap.
 * @param is_vertical True for a vertical cut, false for horizontal.
 * @return A binary mask (CV_8UC1) where 255 indicates pixels to take from the new patch.
 */
static cv::Mat calculateMinErrorCut(const cv::Mat& overlap_new, const cv::Mat& overlap_old, bool is_vertical) {
    if (overlap_new.empty() || overlap_old.empty() || overlap_new.size() != overlap_old.size()) {
        return cv::Mat(overlap_new.size(), CV_8UC1, cv::Scalar(255));
    }

    cv::Mat err;
    cv::absdiff(overlap_new, overlap_old, err);
    cv::transform(err, err, cv::Matx13f(0.299f, 0.587f, 0.114f));
    err.convertTo(err, CV_32F);

    if (!is_vertical) {
        cv::transpose(err, err);
    }

    cv::Mat M = cv::Mat::zeros(err.size(), CV_32F);
    err.row(0).copyTo(M.row(0));

    for (int i = 1; i < err.rows; ++i) {
        for (int j = 0; j < err.cols; ++j) {
            float min_prev = M.at<float>(i - 1, j);
            if (j > 0) min_prev = std::min(min_prev, M.at<float>(i - 1, j - 1));
            if (j < err.cols - 1) min_prev = std::min(min_prev, M.at<float>(i - 1, j + 1));
            M.at<float>(i, j) = err.at<float>(i, j) + min_prev;
        }
    }

    cv::Mat mask = cv::Mat::zeros(err.size(), CV_8UC1);
    double min_val;
    cv::Point min_loc;
    cv::minMaxLoc(M.row(err.rows - 1), &min_val, nullptr, &min_loc, nullptr);
    int current_j = min_loc.x;

    for (int i = err.rows - 1; i >= 0; --i) {
        if (current_j >= 0 && current_j < mask.cols) {
            mask(cv::Rect(current_j, i, mask.cols - current_j, 1)).setTo(255);
        }
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

    if (!is_vertical) {
        cv::transpose(mask, mask);
    }
    return mask;
}

/**
 * @brief Finds the best matching patch using Saliency-guided sampling and combined error.
 */
static cv::Mat findBestMatch(const cv::Mat& target_patch_lab, const cv::Mat& overlap_mask, const cv::Mat& synthesized_region_lab,
                             const cv::Mat& source_lab, const cv::Mat& source_grad_l, const cv::Mat& source_saliency,
                             double alpha, double beta, int patch_size)
{
    static thread_local cv::RNG rng(cv::getTickCount());

    int y_range = source_lab.rows - patch_size;
    int x_range = source_lab.cols - patch_size;
    if (y_range < 0 || x_range < 0) return cv::Mat();

    std::vector<cv::Mat> target_channels;
    cv::split(target_patch_lab, target_channels);
    cv::Mat target_grad_l;
    cv::Sobel(target_channels[0], target_grad_l, CV_32F, 1, 1);

    int num_candidates = 500; // Reduced candidates, as sampling is smarter now
    std::vector<cv::Rect> candidate_rects;
    std::vector<double> candidate_errors;

    // Saliency-guided sampling
    cv::Mat saliency_prob;
    cv::normalize(source_saliency, saliency_prob, 1.0, 0.0, cv::NORM_MINMAX, CV_32F);
    saliency_prob += 0.1; // Add a small constant to allow sampling from non-salient regions

    for (int i = 0; i < num_candidates; ++i) {
        cv::Point sample_point;
        cv::minMaxLoc(saliency_prob, nullptr, nullptr, nullptr, &sample_point); // A simple way to get a high-saliency point
        int y = rng.uniform(0, y_range);
        int x = rng.uniform(0, x_range);
        // Give a 50% chance to pick a random point vs. a salient point
        if (rng.uniform(0,10) > 5) {
            y = std::min(y_range -1, std::max(0, sample_point.y - patch_size/2));
            x = std::min(x_range -1, std::max(0, sample_point.x - patch_size/2));
        }

        cv::Rect source_rect(x, y, patch_size, patch_size);
        cv::Rect crop_rect(0, 0, target_patch_lab.cols, target_patch_lab.rows);

        double boundary_error = 0;
        if (cv::countNonZero(overlap_mask) > 0) {
            cv::Mat source_patch_lab_cropped = source_lab(source_rect)(crop_rect);
            cv::Mat diff_boundary;
            cv::absdiff(source_patch_lab_cropped, synthesized_region_lab, diff_boundary);
            cv::multiply(diff_boundary, diff_boundary, diff_boundary);
            std::vector<cv::Mat> diff_channels;
            cv::split(diff_boundary, diff_channels);
            diff_channels[0] = diff_channels[0].mul(overlap_mask, 1.0/255.0);
            diff_channels[1] = diff_channels[1].mul(overlap_mask, 1.0/255.0);
            diff_channels[2] = diff_channels[2].mul(overlap_mask, 1.0/255.0);
            cv::Scalar ssd_boundary = cv::sum(diff_channels[0]) + cv::sum(diff_channels[1]) + cv::sum(diff_channels[2]);
            boundary_error = ssd_boundary[0] / (cv::countNonZero(overlap_mask) + 1e-6);
        }

        cv::Mat source_patch_lab_cropped = source_lab(source_rect)(crop_rect);
        std::vector<cv::Mat> source_channels;
        cv::split(source_patch_lab_cropped, source_channels);
        cv::Mat diff_lum;
        cv::absdiff(source_channels[0], target_channels[0], diff_lum);
        diff_lum.convertTo(diff_lum, CV_32F);
        cv::multiply(diff_lum, diff_lum, diff_lum);
        double lum_error = cv::mean(diff_lum)[0];

        cv::Mat source_grad_l_cropped = source_grad_l(source_rect)(crop_rect);
        cv::Mat diff_grad;
        cv::absdiff(source_grad_l_cropped, target_grad_l, diff_grad);
        double grad_error = cv::mean(diff_grad)[0];

        double content_error = (1.0 - beta) * lum_error + beta * grad_error;
        double total_error = alpha * boundary_error + (1.0 - alpha) * content_error;
        candidate_rects.push_back(source_rect);
        candidate_errors.push_back(total_error);
    }

    if (candidate_errors.empty()) return cv::Mat();

    double min_error = *std::min_element(candidate_errors.begin(), candidate_errors.end());
    std::vector<cv::Rect> tolerated_candidates;
    double tolerance = 1.2;
    for (size_t i = 0; i < candidate_errors.size(); ++i) {
        if (candidate_errors[i] <= min_error * tolerance) {
            tolerated_candidates.push_back(candidate_rects[i]);
        }
    }

    if (tolerated_candidates.empty()) {
        auto min_it = std::min_element(candidate_errors.begin(), candidate_errors.end());
        size_t min_idx = std::distance(candidate_errors.begin(), min_it);
        return source_lab(candidate_rects[min_idx]).clone();
    }

    int random_idx = rng.uniform(0, (int)tolerated_candidates.size());
    return source_lab(tolerated_candidates[random_idx]).clone();
}

// --- 核心处理函数 ---

QImage ImageTextureTransferProcessor::process(const QImage &contentImage, const QImage &textureImage)
{
    try {
        if (contentImage.isNull() || textureImage.isNull()) return QImage();

        cv::Mat content_mat = ImageConverter::qImageToMat(contentImage);
        cv::Mat texture_mat = ImageConverter::qImageToMat(textureImage);

        // --- 1. Build Pyramids and Pre-calculate Features ---
        int num_levels = 4;
        std::vector<cv::Mat> content_pyramid_lab, texture_pyramid_lab;
        cv::Mat content_lab, texture_lab;
        cv::cvtColor(content_mat, content_lab, cv::COLOR_BGR2Lab);
        cv::cvtColor(texture_mat, texture_lab, cv::COLOR_BGR2Lab);
        cv::buildPyramid(content_lab, content_pyramid_lab, num_levels);
        cv::buildPyramid(texture_lab, texture_pyramid_lab, num_levels);

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

        cv::Mat result_lab;

        // --- 2. Coarse-to-Fine Synthesis through Pyramid Levels ---
        for (int level = num_levels; level >= 0; --level) {
            qDebug() << "--- Processing Pyramid Level" << level << "---";
            const cv::Mat& current_content_lab = content_pyramid_lab[level];
            const cv::Mat& current_texture_lab = texture_pyramid_lab[level];
            const cv::Mat& current_texture_grad_l = texture_pyramid_grad_l[level];
            const cv::Mat& current_texture_saliency = texture_pyramid_saliency[level];

            if (level == num_levels) {
                result_lab = cv::Mat::zeros(current_content_lab.size(), current_content_lab.type());
            } else {
                cv::pyrUp(result_lab, result_lab, current_content_lab.size());
            }

            int patch_size = std::max(5, std::min(current_content_lab.rows, current_content_lab.cols) / 8);
            if (patch_size % 2 == 0) patch_size++;
            if (patch_size >= current_texture_lab.rows || patch_size >= current_texture_lab.cols) continue;

            int overlap = std::max(1, patch_size / 6);
            double alpha = 0.1 + 0.8 * (level / (double)num_levels); // More boundary focus on finer levels
            double beta = 0.7;

            for (int y = 0; y < current_content_lab.rows; y += patch_size - overlap) {
                for (int x = 0; x < current_content_lab.cols; x += patch_size - overlap) {
                    int w = std::min(patch_size, current_content_lab.cols - x);
                    int h = std::min(patch_size, current_content_lab.rows - y);
                    if (w <= overlap || h <= overlap) continue;
                    cv::Rect current_rect(x, y, w, h);

                    cv::Mat target_patch = current_content_lab(current_rect);
                    cv::Mat synthesized_region = result_lab(current_rect);
                    cv::Mat overlap_mask = cv::Mat::zeros(h, w, CV_8UC1);

                    if (x > 0) overlap_mask(cv::Rect(0, 0, std::min(overlap, w), h)).setTo(255);
                    if (y > 0) overlap_mask(cv::Rect(0, 0, w, std::min(overlap, h))).setTo(255);

                    cv::Mat best_match_full = findBestMatch(target_patch, overlap_mask, synthesized_region, current_texture_lab, current_texture_grad_l, current_texture_saliency, alpha, beta, patch_size);
                    if (best_match_full.empty()) continue;

                    cv::Mat best_match = best_match_full(cv::Rect(0, 0, w, h));
                    cv::Mat final_mask = cv::Mat(h, w, CV_8UC1, cv::Scalar(255));

                    if (x > 0) {
                        cv::Rect rect(0, 0, std::min(overlap, w), h);
                        cv::Mat cut_mask = calculateMinErrorCut(best_match(rect), synthesized_region(rect), true);
                        cut_mask.copyTo(final_mask(rect));
                    }
                    if (y > 0) {
                        cv::Rect rect(0, 0, w, std::min(overlap, h));
                        cv::Mat cut_mask = calculateMinErrorCut(best_match(rect), synthesized_region(rect), false);
                        cv::bitwise_and(final_mask(rect), cut_mask, final_mask(rect));
                    }
                    best_match.copyTo(result_lab(current_rect), final_mask);
                }
            }
        }

        // --- 3. Final Color Preservation Step ---
        qDebug() << "Performing final color preservation...";
        std::vector<cv::Mat> result_channels, original_content_channels;
        cv::split(result_lab, result_channels);
        cv::split(content_lab, original_content_channels);

        std::vector<cv::Mat> final_channels = { result_channels[0], original_content_channels[1], original_content_channels[2] };
        cv::merge(final_channels, result_lab);
        qDebug() << "Color preservation complete.";

        cv::Mat result_bgr;
        cv::cvtColor(result_lab, result_bgr, cv::COLOR_Lab2BGR);
        return ImageConverter::matToQImage(result_bgr);

    } catch (const cv::Exception& e) {
        qDebug() << "!!!!!!!!!! An OpenCV Exception was caught in Texture Transfer !!!!!!!!!!";
        qDebug() << "Error Message:" << e.msg.c_str();
        return QImage();
    }
}
