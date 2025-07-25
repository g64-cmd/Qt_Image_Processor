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

#ifndef DROPPABLEGRAPHICSVIEW_H
#define DROPPABLEGRAPHICSVIEW_H

// =============================================================================
// File: droppablegraphicsview.h
//
// Description:
// 该文件定义了 DroppableGraphicsView 类，这是一个继承自 QGraphicsView
// 的自定义视图，专门用于支持拖放（Drag and Drop）操作和鼠标移动事件。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QGraphicsView>

/**
 * @class DroppableGraphicsView
 * @brief 一个支持接收拖放数据和报告鼠标移动的 QGraphicsView。
 *
 * 通过重写拖放相关的事件处理器（dragEnterEvent, dragMoveEvent, dropEvent），
 * 该视图可以接收从 DraggableItemModel 拖出的自定义数据。
 * 同时，它也重写了 mouseMoveEvent 来发射一个包含场景坐标的信号。
 */
class DroppableGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     * @param parent 父窗口部件，默认为nullptr。
     */
    explicit DroppableGraphicsView(QWidget *parent = nullptr);

signals:
    /**
     * @brief 当一个有效的暂存区图像被放置到此视图上时，发射此信号。
     * @param imageId 被放置的图像的唯一ID。
     */
    void stagedImageDropped(const QString &imageId);

    /**
     * @brief 当鼠标在此视图上移动时，发射此信号。
     * @param scenePos 鼠标指针在场景坐标系中的当前位置。
     */
    void mouseMovedOnScene(const QPointF &scenePos);

protected:
    /**
     * @brief 当拖动进入此控件时被调用。
     *
     * 检查拖动的数据是否包含我们支持的MIME类型，如果支持则接受事件。
     * @param event 拖动进入事件。
     */
    void dragEnterEvent(QDragEnterEvent *event) override;

    /**
     * @brief 当拖动在此控件内移动时被调用。
     *
     * 持续接受事件，以向用户显示可以放置的光标。
     * @param event 拖动移动事件。
     */
    void dragMoveEvent(QDragMoveEvent *event) override;

    /**
     * @brief 当放置操作发生时被调用。
     *
     * 解析MIME数据，提取图像ID，并发射 stagedImageDropped 信号。
     * @param event 放置事件。
     */
    void dropEvent(QDropEvent *event) override;

    /**
     * @brief 当鼠标在此控件上移动时被调用。
     *
     * 获取鼠标位置，将其转换为场景坐标，并发射 mouseMovedOnScene 信号。
     * @param event 鼠标事件。
     */
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // DROPPABLEGRAPHICSVIEW_H
