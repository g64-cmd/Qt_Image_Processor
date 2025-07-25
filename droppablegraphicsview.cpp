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
// File: droppablegraphicsview.cpp
//
// Description:
// DroppableGraphicsView 类的实现文件。该文件实现了处理拖放事件和
// 鼠标移动事件的逻辑。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "droppablegraphicsview.h"
#include <QDragEnterEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QDataStream>
#include <QIODevice>

/**
 * @brief DroppableGraphicsView 构造函数。
 * @param parent 父窗口部件。
 */
DroppableGraphicsView::DroppableGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    // 启用放置（drop）事件。没有这行，该控件不会接收任何拖放事件。
    setAcceptDrops(true);
    // 启用鼠标跟踪。这使得即使没有按下鼠标按钮，mouseMoveEvent也会被触发。
    setMouseTracking(true);
}

/**
 * @brief 当拖动进入此控件时被调用。
 *
 * 检查拖动的数据是否包含我们自定义的MIME类型 "application/x-draggable-item"。
 * 如果是，则接受事件，向用户表明这是一个有效的放置目标。
 * @param event 拖动进入事件。
 */
void DroppableGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-draggable-item")) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

/**
 * @brief 当拖动在此控件内移动时被调用。
 *
 * 与 dragEnterEvent 类似，持续检查MIME类型并接受事件，
 * 以便在整个拖动过程中保持有效的放置光标。
 * @param event 拖动移动事件。
 */
void DroppableGraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-draggable-item")) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

/**
 * @brief 当放置操作发生时被调用。
 *
 * 这是处理拖放逻辑的核心。它会解析接收到的MIME数据，
 * 提取出我们之前存入的图像ID，并发射信号通知其他部分。
 * @param event 放置事件。
 */
void DroppableGraphicsView::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasFormat("application/x-draggable-item")) {
        // 1. 获取与自定义MIME类型关联的数据
        QByteArray itemData = mimeData->data("application/x-draggable-item");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

        // 2. 从数据流中反序列化出图像ID
        QString imageId;
        dataStream >> imageId;

        // 3. 发射信号，将提取出的ID传递出去
        emit stagedImageDropped(imageId);

        // 4. 接受放置操作
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

/**
 * @brief 当鼠标在此控件上移动时被调用。
 *
 * 由于在构造函数中设置了 setMouseTracking(true)，此事件会持续触发。
 * @param event 鼠标事件。
 */
void DroppableGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    // 将鼠标在控件坐标系中的位置 (event->pos()) 映射到场景坐标系
    QPointF scenePos = mapToScene(event->pos());
    // 发射信号，将场景坐标传递出去，供颜色拾取器等功能使用
    emit mouseMovedOnScene(scenePos);

    // 调用基类的实现，以确保视图的默认行为（如拖动视图）能够正常工作
    QGraphicsView::mouseMoveEvent(event);
}
