#include "stagingareamanager.h"
#include <QStandardItemModel>
#include <QIcon>
#include <QUuid>
#include <utility>

StagingAreaManager::StagingAreaManager(QStandardItemModel *model, QObject *parent)
    : QObject(parent), model(model)
{
    Q_ASSERT(model != nullptr);
}

QString StagingAreaManager::addNewImage(const QPixmap &pixmap, const QString &baseName)
{
    if (pixmap.isNull()) return QString();

    StagedImage newImage;
    newImage.id = QUuid::createUuid().toString();
    newImage.pixmap = pixmap;
    newImage.name = QString("%1_%2").arg(baseName).arg(++imageCounter);

    stagedImages.prepend(newImage);

    while (stagedImages.size() > MaxStagedImages) {
        stagedImages.removeLast();
    }

    updateModel();
    return newImage.id;
}

void StagingAreaManager::updateImage(const QString &id, const QPixmap &newPixmap)
{
    int foundIndex = -1;
    for (int i = 0; i < stagedImages.size(); ++i) {
        if (stagedImages[i].id == id) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex == -1) return;

    StagedImage item = stagedImages.takeAt(foundIndex);
    item.pixmap = newPixmap;
    stagedImages.prepend(item);

    updateModel();
}

void StagingAreaManager::promoteImage(const QString &id)
{
    int foundIndex = -1;
    for (int i = 0; i < stagedImages.size(); ++i) {
        if (stagedImages[i].id == id) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex <= 0) return;

    StagedImage item = stagedImages.takeAt(foundIndex);
    stagedImages.prepend(item);

    updateModel();
}

QPixmap StagingAreaManager::getPixmap(const QString &id) const
{
    for (const auto &image : std::as_const(stagedImages)) {
        if (image.id == id) {
            return image.pixmap;
        }
    }
    return QPixmap();
}

// --- 新增函数实现 ---
int StagingAreaManager::getImageCount() const
{
    return stagedImages.size();
}


void StagingAreaManager::updateModel()
{
    if (!model) return;

    model->clear();
    for (const auto &image : std::as_const(stagedImages)) {
        QStandardItem *item = new QStandardItem();
        item->setIcon(QIcon(image.pixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        item->setText(image.name);
        item->setData(image.id, Qt::UserRole);
        model->appendRow(item);
    }
}
