// =============================================================================
//
// Copyright (C) 2025 g64-cmd
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// =============================================================================

// =============================================================================
// File: draggableitemmodel.cpp
//
// Description:
// DraggableItemModel 类的实现文件。该文件实现了支持拖拽操作
// 所需的几个关键虚函数。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "draggableitemmodel.h"
#include <QMimeData>
#include <QDataStream>
#include <QIODevice>

/**
 * @brief DraggableItemModel 构造函数。
 * @param parent 父对象。
 */
DraggableItemModel::DraggableItemModel(QObject *parent)
    : QStandardItemModel(parent)
{}

/**
 * @brief 返回指定索引项的标志（flags）。
 *
 * 通过在默认标志的基础上添加 Qt::ItemIsDragEnabled，我们告诉视图
 * (View) 这个项是可以被拖动的。
 * @param index 要查询的模型索引。
 * @return 返回该项的标志集合。
 */
Qt::ItemFlags DraggableItemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QStandardItemModel::flags(index);
    if (index.isValid()) {
        // 如果索引有效，则在默认标志的基础上增加“可拖动”标志
        return Qt::ItemIsDragEnabled | defaultFlags;
    }
    return defaultFlags;
}

/**
 * @brief 返回此模型可以提供的MIME类型列表。
 *
 * MIME类型是拖放框架用来识别数据格式的字符串。我们定义一个自定义的
 * MIME类型 "application/x-draggable-item"，以便在放置(drop)时，
 * 接收方可以确认这是我们自己的数据。
 * @return 包含自定义MIME类型的字符串列表。
 */
QStringList DraggableItemModel::mimeTypes() const
{
    QStringList types;
    types << "application/x-draggable-item";
    return types;
}

/**
 * @brief 将指定索引的数据打包成QMimeData对象。
 *
 * 当拖拽操作开始时，视图会调用此函数来获取要传输的数据。
 * 我们从模型项的 UserRole 中获取图像的唯一ID，将其序列化后
 * 存入 QMimeData 对象中。
 * @param indexes 被拖拽的项的索引列表。
 * @return 返回一个包含自定义数据的 QMimeData 指针。
 */
QMimeData *DraggableItemModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty()) {
        return nullptr;
    }

    // 从第一个被拖动的项中获取数据。我们假设只拖动一个项。
    // 数据存储在 Qt::UserRole 中，这里是图像的唯一ID。
    QString imageId = indexes.first().data(Qt::UserRole).toString();
    if (imageId.isEmpty()) {
        return nullptr;
    }

    // 创建MIME数据对象
    QMimeData *mimeData = new QMimeData();

    // 将图像ID序列化为 QByteArray
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    stream << imageId;

    // 将序列化后的数据与我们自定义的MIME类型关联起来
    mimeData->setData("application/x-draggable-item", encodedData);

    return mimeData;
}
