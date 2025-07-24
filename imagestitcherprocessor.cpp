#include "imagestitcherprocessor.h"
#include <opencv2/stitching.hpp>
#include <QDebug>

ImageStitcherProcessor::ImageStitcherProcessor() {}

cv::Mat ImageStitcherProcessor::process(const std::vector<cv::Mat> &images)
{
    if (images.size() < 2) {
        qWarning() << "需要至少两张图片才能拼接。";
        return cv::Mat();
    }
    cv::Mat pano;
    cv::Ptr<cv::Stitcher> stitcher = cv::Stitcher::create(cv::Stitcher::PANORAMA);
    cv::Stitcher::Status status = stitcher->stitch(images, pano);
    if (status != cv::Stitcher::OK) {
        qWarning() << "无法拼接，错误代码：" << int(status);
        return cv::Mat();
    }
    qDebug() << "拼接成功。";
    return pano;
}
