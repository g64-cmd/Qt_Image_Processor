#include "imageconverter.h"

QImage ImageConverter::matToQImage(const cv::Mat &mat)
{
    // 根据 Mat 类型选择合适的 QImage 格式
    switch (mat.type()) {
    // 8位单通道 (灰度图)
    case CV_8UC1: {
        QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8);
        return image.copy(); // 返回一个深拷贝
    }

    // 8位三通道 (BGR)
    case CV_8UC3: {
        // OpenCV 的颜色顺序是 BGR，而 Qt 是 RGB，所以需要转换
        QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_RGB888);
        return image.rgbSwapped(); // 返回一个颜色通道交换后的深拷贝
    }

    // 8位四通道 (BGRA)
    case CV_8UC4: {
        QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_ARGB32);
        return image.copy();
    }

    // 其他不支持的格式
    default:
        break;
    }

    return QImage(); // 返回一个空的 QImage
}

cv::Mat ImageConverter::qImageToMat(const QImage &image)
{
    cv::Mat mat;
    // 根据 QImage 格式选择合适的 Mat 类型
    switch (image.format()) {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()), image.bytesPerLine());
        break;

    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, const_cast<uchar*>(image.bits()), image.bytesPerLine());
        // OpenCV 需要 BGR，所以需要从 RGB 转换
        cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
        break;

    case QImage::Format_Grayscale8:
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, const_cast<uchar*>(image.bits()), image.bytesPerLine());
        break;

    // 其他格式可以先转换为支持的格式
    default:
        QImage temp = image.convertToFormat(QImage::Format_ARGB32);
        mat = cv::Mat(temp.height(), temp.width(), CV_8UC4, const_cast<uchar*>(temp.bits()), temp.bytesPerLine());
        break;
    }

    return mat.clone(); // 返回一个深拷贝
}
