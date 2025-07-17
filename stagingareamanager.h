#ifndef STAGINGAREAMANAGER_H
#define STAGINGAREAMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QList>
#include <QPair>

class QStandardItemModel;

/**
 * @brief 图片暂存区管理器
 *
 * 负责管理用户处理过程中的临时图片 (QPixmap)。
 */
class StagingAreaManager : public QObject
{
    Q_OBJECT
public:
    struct StagedImage {
        QString id;
        QString name;
        QPixmap pixmap;
    };

    explicit StagingAreaManager(QStandardItemModel *model, QObject *parent = nullptr);

    /**
     * @brief 添加一张全新的图片到暂存区 (通常来自文件)
     * @return 新图片的唯一ID
     */
    QString addNewImage(const QPixmap &pixmap, const QString &baseName);

    /**
     * @brief 更新暂存区中现有图片的数据 (例如，在应用滤镜后)
     * @param id 要更新的图片的ID
     * @param newPixmap 新的图片数据
     */
    void updateImage(const QString &id, const QPixmap &newPixmap);

    /**
     * @brief 将指定的图片提升到列表顶部 (例如，在被点击或拖动后)
     * @param id 要提升的图片的ID
     */
    void promoteImage(const QString &id);

    QPixmap getPixmap(const QString &id) const;

private:
    void updateModel();

    QStandardItemModel *model;
    QList<StagedImage> stagedImages;
    static const int MaxStagedImages = 15;
    int imageCounter = 0;
};

#endif // STAGINGAREAMANAGER_H
