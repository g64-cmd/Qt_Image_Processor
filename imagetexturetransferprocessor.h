#ifndef IMAGETEXTURETRANSFERPROCESSOR_H
#define IMAGETEXTURETRANSFERPROCESSOR_H

#include <QImage>

/**
 * @brief 图像纹理迁移功能处理器
 *
 * 遵循单一职责原则，使用经典的颜色统计量匹配算法。
 */
class ImageTextureTransferProcessor
{
public:
    ImageTextureTransferProcessor() = delete;

    /**
     * @brief 对外提供的唯一处理接口
     * @param contentImage 内容图片 (QImage)
     * @param textureImage 纹理/风格图片 (QImage)
     * @return 迁移了纹理的新 QImage
     */
    static QImage process(const QImage &contentImage, const QImage &textureImage);
};

#endif // IMAGETEXTURETRANSFERPROCESSOR_H
