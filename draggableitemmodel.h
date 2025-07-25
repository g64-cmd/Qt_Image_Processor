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

#ifndef DRAGGABLEITEMMODEL_H
#define DRAGGABLEITEMMODEL_H

// =============================================================================
// File: draggableitemmodel.h
//
// Description:
// 该文件定义了 DraggableItemModel 类，这是一个继承自 QStandardItemModel
// 的自定义数据模型，专门用于支持拖拽（Drag and Drop）操作。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QStandardItemModel>

class QMimeData;

/**
 * @class DraggableItemModel
 * @brief 一个支持拖拽操作的数据模型。
 *
 * 通过重写 flags(), mimeTypes(), 和 mimeData() 函数，
 * 该模型能够在其所附加的视图（如 QListView）中启用拖拽功能，
 * 并在拖动时能够正确地打包自定义数据（如图像ID）。
 */
class DraggableItemModel : public QStandardItemModel
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     * @param parent 父对象，默认为nullptr。
     */
    explicit DraggableItemModel(QObject *parent = nullptr);

    /**
     * @brief 返回指定索引项的标志（flags）。
     *
     * 重写此函数以启用拖拽。除了默认标志外，还添加了 Qt::ItemIsDragEnabled。
     * @param index 要查询的模型索引。
     * @return 返回该项的标志集合。
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * @brief 返回此模型可以提供的MIME类型列表。
     *
     * 重写此函数以声明一个自定义的MIME类型，用于在拖放操作中识别数据。
     * @return 包含自定义MIME类型的字符串列表。
     */
    QStringList mimeTypes() const override;

    /**
     * @brief 将指定索引的数据打包成QMimeData对象。
     *
     * 当拖拽操作开始时，此函数被调用。它将项的自定义数据（通常是唯一ID）
     * 存入QMimeData对象，以便在放置（drop）时可以被正确解析。
     * @param indexes 被拖拽的项的索引列表。
     * @return 返回一个包含自定义数据的 QMimeData 指针。
     */
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
};

#endif // DRAGGABLEITEMMODEL_H
