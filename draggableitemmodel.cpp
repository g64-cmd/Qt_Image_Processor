#include "draggableitemmodel.h"
#include "droppablegraphicsview.h" // <-- 关键修复：包含头文件以获取 extern 常量声明
#include <QMimeData>

DraggableItemModel::DraggableItemModel(QObject *parent)
    : QStandardItemModel(parent)
{}

QMimeData *DraggableItemModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty()) {
        return nullptr;
    }

    QMimeData *mimeData = new QMimeData();
    QString imageId = indexes.first().data(Qt::UserRole).toString();

    if (!imageId.isEmpty()) {
        // 使用在别处定义的全局常量
        mimeData->setData(STAGED_IMAGE_MIME_TYPE, imageId.toUtf8());
    }

    return mimeData;
}
