#include "draggableitemmodel.h"
#include <QMimeData>
#include <QDataStream>
#include <QIODevice>

DraggableItemModel::DraggableItemModel(QObject *parent)
    : QStandardItemModel(parent)
{}

Qt::ItemFlags DraggableItemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QStandardItemModel::flags(index);
    if (index.isValid()) {
        return Qt::ItemIsDragEnabled | defaultFlags;
    }
    return defaultFlags;
}

QStringList DraggableItemModel::mimeTypes() const
{
    QStringList types;
    types << "application/x-draggable-item";
    return types;
}

QMimeData *DraggableItemModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty()) {
        return nullptr;
    }
    QString imageId = indexes.first().data(Qt::UserRole).toString();
    if (imageId.isEmpty()) {
        return nullptr;
    }
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    stream << imageId;
    mimeData->setData("application/x-draggable-item", encodedData);
    return mimeData;
}
