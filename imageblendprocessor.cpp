#include "imageblendprocessor.h"
#include "imageconverter.h"
#include <opencv2/opencv.hpp>

QImage ImageBlendProcessor::process(const QImage &imageA, const QImage &imageB, double alpha)
{
    if (imageA.isNull() || imageB.isNull()) {
        return QImage();
    }
    cv::Mat matA = ImageConverter::qImageToMat(imageA);
    cv::Mat matB = ImageConverter::qImageToMat(imageB);
    if (matA.empty() || matB.empty()) {
        return QImage();
    }
    if (matA.size() != matB.size()) {
        cv::resize(matB, matB, matA.size());
    }
    if (matA.type() != matB.type()) {
        matB.convertTo(matB, matA.type());
    }
    if (matA.channels() != matB.channels()) {
        int code = (matA.channels() == 3) ? cv::COLOR_BGRA2BGR : cv::COLOR_BGR2BGRA;
        if (matA.channels() > matB.channels()) {
        } else {
            cv::cvtColor(matB, matB, code);
        }
    }
    cv::Mat resultMat;
    double beta = 1.0 - alpha;
    cv::addWeighted(matA, beta, matB, alpha, 0.0, resultMat);
    return ImageConverter::matToQImage(resultMat);
}
