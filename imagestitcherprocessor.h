// imagestitcherprocessor.h
#ifndef IMAGESTITCHERPROCESSOR_H
#define IMAGESTITCHERPROCESSOR_H

#include <opencv2/opencv.hpp>
#include <vector>

class ImageStitcherProcessor
{
public:
    ImageStitcherProcessor();
    cv::Mat process(const std::vector<cv::Mat> &images);
};

#endif // IMAGESTITCHERPROCESSOR_H
