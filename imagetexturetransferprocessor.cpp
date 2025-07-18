#include "imagetexturetransferprocessor.h"
#include "imageconverter.h"
#include <opencv2/opencv.hpp>
#include <vector>

// 辅助函数：计算并应用直方图匹配的查找表 (LUT)
void matchHistogram(cv::Mat &src, const cv::Mat &dst, cv::Mat &result)
{
    // 计算源图像和目标图像的直方图
    int histSize = 256;
    float range[] = { 0, 256 };
    const float* histRange = { range };
    cv::Mat srcHist, dstHist;
    cv::calcHist(&src, 1, 0, cv::Mat(), srcHist, 1, &histSize, &histRange, true, false);
    cv::calcHist(&dst, 1, 0, cv::Mat(), dstHist, 1, &histSize, &histRange, true, false);

    // 计算累积分布函数 (CDF)
    cv::Mat srcCDF = srcHist.clone();
    cv::Mat dstCDF = dstHist.clone();
    for (int i = 1; i < histSize; i++) {
        srcCDF.at<float>(i) += srcCDF.at<float>(i - 1);
        dstCDF.at<float>(i) += dstCDF.at<float>(i - 1);
    }

    // 归一化 CDF
    cv::normalize(srcCDF, srcCDF, 0, 255, cv::NORM_MINMAX);
    cv::normalize(dstCDF, dstCDF, 0, 255, cv::NORM_MINMAX);

    // 创建查找表 (LUT)
    cv::Mat lut(1, 256, CV_8U);
    int j = 0;
    for (int i = 0; i < histSize; i++) {
        // 找到源CDF中的值在目标CDF中最接近的位置
        while (j < histSize && dstCDF.at<float>(j) < srcCDF.at<float>(i)) {
            j++;
        }
        lut.at<uchar>(i) = static_cast<uchar>(j);
    }

    // 应用查找表
    cv::LUT(src, lut, result);
}

QImage ImageTextureTransferProcessor::process(const QImage &contentImage, const QImage &textureImage)
{
    if (contentImage.isNull() || textureImage.isNull()) {
        return QImage();
    }

    // 1. 将 QImage 转换为 cv::Mat
    cv::Mat contentMat = ImageConverter::qImageToMat(contentImage);
    cv::Mat textureMat = ImageConverter::qImageToMat(textureImage);

    if (contentMat.empty() || textureMat.empty()) {
        return QImage();
    }

    // 2. 将 BGR 图像转换为 Lab 色彩空间
    cv::Mat contentLab, textureLab;
    cv::cvtColor(contentMat, contentLab, cv::COLOR_BGR2Lab);
    cv::cvtColor(textureMat, textureLab, cv::COLOR_BGR2Lab);

    // 3. 将 Lab 图像的通道分离
    std::vector<cv::Mat> contentChannels, textureChannels;
    cv::split(contentLab, contentChannels);
    cv::split(textureLab, textureChannels);

    // 4. 对每个通道分别进行直方图匹配
    std::vector<cv::Mat> resultChannels;
    for (size_t i = 0; i < contentChannels.size(); ++i) {
        cv::Mat matchedChannel;
        matchHistogram(contentChannels[i], textureChannels[i], matchedChannel);
        resultChannels.push_back(matchedChannel);
    }

    // 5. 合并处理后的通道
    cv::Mat resultLab;
    cv::merge(resultChannels, resultLab);

    // 6. 将 Lab 图像转换回 BGR 色彩空间
    cv::Mat resultBgr;
    cv::cvtColor(resultLab, resultBgr, cv::COLOR_Lab2BGR);

    // 7. 将最终的 cv::Mat 转换回 QImage 并返回
    return ImageConverter::matToQImage(resultBgr);
}
