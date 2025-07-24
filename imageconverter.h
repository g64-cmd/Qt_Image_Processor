#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H

#include <QImage>
#include <opencv2/opencv.hpp>

/**
 * @brief 图像格式转换辅助类
 *
 * 提供静态方法用于在 Qt 的 QImage 和 OpenCV 的 cv::Mat 之间进行转换。
 */
class ImageConverter
{
public:
    ImageConverter() = delete;

    /**
     * @brief 将 cv::Mat 转换为 QImage
     * @param mat 输入的 OpenCV Mat 对象 (必须是 CV_8UC1, CV_8UC3 或 CV_8UC4 类型)
     * @return 转换后的 QImage 对象
     */
    static QImage matToQImage(const cv::Mat &mat);

    /**
     * @brief 将 QImage 转换为 cv::Mat
     * @param image 输入的 QImage 对象
     * @return 转换后的 OpenCV Mat 对象
     */
    static cv::Mat qImageToMat(const QImage &image);
};

#endif // IMAGECONVERTER_H
