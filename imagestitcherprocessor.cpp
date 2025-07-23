// imagestitcherprocessor.cpp
#include "imagestitcherprocessor.h"
#include <opencv2/stitching.hpp>
#include <QDebug>

ImageStitcherProcessor::ImageStitcherProcessor() {}

cv::Mat ImageStitcherProcessor::process(const std::vector<cv::Mat> &images)
{
    if (images.size() < 2) {
        qWarning() << "Need at least 2 images to stitch.";
        return cv::Mat();
    }

    cv::Mat pano;
    // Use OpenCV's high-level Stitcher API
    cv::Ptr<cv::Stitcher> stitcher = cv::Stitcher::create(cv::Stitcher::PANORAMA);

    // The stitcher class internally handles feature detection, matching,
    // homography estimation, warping, and blending.
    cv::Stitcher::Status status = stitcher->stitch(images, pano);

    if (status != cv::Stitcher::OK) {
        qWarning() << "Can't stitch images, error code = " << int(status);
        return cv::Mat();
    }

    qDebug() << "Stitching successful.";
    return pano;
}
