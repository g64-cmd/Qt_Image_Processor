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

#ifndef INTERACTIVEPIXMAPITEM_H
#define INTERACTIVEPIXMAPITEM_H

// =============================================================================
// File: interactivepixmapitem.h
//
// Description:
// 该文件定义了 InteractivePixmapItem 类，这是一个可交互的图形项，
// 用于在 QGraphicsScene 中显示和操作图像。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QGraphicsObject>
#include <QPixmap>

// --- 前置声明 ---
class QGraphicsPixmapItem;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;

/**
 * @class InteractivePixmapItem
 * @brief 一个可交互的 QGraphicsPixmapItem。
 *
 * 继承自 QGraphicsObject 以获得信号和槽的支持。它内部包含一个
 * QGraphicsPixmapItem 来负责实际的图像绘制，而此类本身则负责处理
 * 用户的交互事件，如点击、滚轮缩放等。
 */
class InteractivePixmapItem : public QGraphicsObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     * @param pixmap 要显示的图像。
     * @param parent 父图形项，默认为nullptr。
     */
    explicit InteractivePixmapItem(const QPixmap &pixmap, QGraphicsItem *parent = nullptr);

    /**
     * @brief 返回该项的边界矩形。
     *
     * 这是 QGraphicsItem 的一个关键虚函数，用于碰撞检测、重绘等。
     * @return 该项在局部坐标系下的边界矩形。
     */
    QRectF boundingRect() const override;

    /**
     * @brief 绘制该项的内容。
     * @param painter 用于绘制的 QPainter 对象。
     * @param option 提供样式选项，如状态（选中、悬停等）。
     * @param widget 绘制所在的窗口部件，可选。
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

signals:
    /**
     * @brief 当该项被点击时发射此信号。
     * @param item 指向被点击的 InteractivePixmapItem 实例的指针。
     */
    void itemClicked(InteractivePixmapItem *item);

protected:
    /**
     * @brief 鼠标按下事件处理器。
     *
     * 用于处理项被点击时的逻辑，如提升层级和发射信号。
     * @param event 鼠标场景事件。
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 鼠标滚轮事件处理器。
     *
     * 用于实现通过滚轮缩放图像的功能。
     * @param event 鼠标滚轮场景事件。
     */
    void wheelEvent(QGraphicsSceneWheelEvent *event) override;

private:
    // --- 成员变量 ---
    // 使用组合的方式，让一个标准的QGraphicsPixmapItem负责图像的显示。
    // InteractivePixmapItem 则作为包装器，为其添加交互能力。
    QGraphicsPixmapItem *pixmapItem;
};

#endif // INTERACTIVEPIXMAPITEM_H
