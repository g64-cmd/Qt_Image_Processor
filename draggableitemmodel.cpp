#include "draggableitemmodel.h"
#include <QMimeData>
#include <QDataStream>
#include <QIODevice>

DraggableItemModel::DraggableItemModel(QObject *parent)
    : QStandardItemModel(parent)
{}

Qt::ItemFlags DraggableItemModel::flags(const QModelIndex &index) const
{
    // 获取默认标志
    Qt::ItemFlags defaultFlags = QStandardItemModel::flags(index);

    // 如果索引有效，则添加可拖动标志
    if (index.isValid()) {
        return Qt::ItemIsDragEnabled | defaultFlags;
    }

    return defaultFlags;
}

QStringList DraggableItemModel::mimeTypes() const
{
    // 声明我们自定义的MIME类型
    QStringList types;
    types << "application/x-draggable-item";
    return types;
}

QMimeData *DraggableItemModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty()) {
        return nullptr;
    }

    // 从第一个被拖动的项中获取我们的自定义数据（imageId）
    // 我们假设 imageId 存储在 UserRole 中
    QString imageId = indexes.first().data(Qt::UserRole).toString();
    if (imageId.isEmpty()) {
        return nullptr;
    }

    // 创建MIME数据对象
    QMimeData *mimeData = new QMimeData();

    // 将 imageId 序列化到 QByteArray 中
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    stream << imageId;

    // 设置MIME数据
    // 这里的 "application/x-draggable-item" 必须与 mimeTypes() 和
    // 接收方（DroppableGraphicsView）检查的类型一致。
    mimeData->setData("application/x-draggable-item", encodedData);

    return mimeData;
}
