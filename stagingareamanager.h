#ifndef STAGINGAREAMANAGER_H
#define STAGINGAREAMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QList>
#include <QPair>

class DraggableItemModel;

class StagingAreaManager : public QObject
{
    Q_OBJECT

public:
    struct StagedImage {
        QString id;
        QString name;
        QPixmap pixmap;
    };

    explicit StagingAreaManager(DraggableItemModel *model, QObject *parent = nullptr);
    QString addNewImage(const QPixmap &pixmap, const QString &baseName);
    void updateImage(const QString &id, const QPixmap &newPixmap);
    void promoteImage(const QString &id);
    QPixmap getPixmap(const QString &id) const;
    StagedImage getStagedImage(const QString &id) const; // Added for name retrieval
    int getImageCount() const;
    void removeImage(const QString &id);

private:
    void updateModel();
    DraggableItemModel *model;
    QList<StagedImage> stagedImages;
    static const int MaxStagedImages = 15;
    int imageCounter = 0;
};

#endif // STAGINGAREAMANAGER_H
