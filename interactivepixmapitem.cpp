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
// File: interactivepixmapitem.cpp
//
// Description:
// InteractivePixmapItem 类的实现文件。该文件包含了可交互图形项的
// 具体实现逻辑。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "interactivepixmapitem.h"
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QtMath>

/**
 * @brief InteractivePixmapItem 构造函数。
 *
 * 负责设置项的交互标志，创建内部的 QGraphicsPixmapItem，并设置变换原点。
 * @param pixmap 要显示的图像。
 * @param parent 父图形项。
 */
InteractivePixmapItem::InteractivePixmapItem(const QPixmap &pixmap, QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    // --- 1. 设置交互标志 ---
    // ItemIsSelectable: 项可以被选中。
    // ItemIsMovable: 项可以被鼠标拖动。
    // ItemSendsGeometryChanges: 当项的位置、变换等几何属性改变时，会发送通知。
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    // 启用悬停事件，以便在需要时可以改变光标样式或显示额外信息。
    setAcceptHoverEvents(true);

    // --- 2. 创建并设置子项 ---
    // 将一个标准的QGraphicsPixmapItem作为子项，它负责绘制图像。
    // 这种组合模式将绘制和交互逻辑分离。
    pixmapItem = new QGraphicsPixmapItem(pixmap, this);

    // --- 3. 设置变换原点 ---
    // 将旋转和缩放的中心点设置在图像的几何中心，确保变换行为符合直觉。
    QSizeF originalSize = pixmap.size();
    setTransformOriginPoint(originalSize.width() / 2, originalSize.height() / 2);
}

/**
 * @brief 返回该项的边界矩形。
 *
 * 直接返回内部 pixmapItem 的边界矩形，因为实际内容是由它定义的。
 * @return 该项在局部坐标系下的边界矩形。
 */
QRectF InteractivePixmapItem::boundingRect() const
{
    return pixmapItem->boundingRect();
}

/**
 * @brief 绘制该项的内容。
 *
 * 这个函数只负责绘制附加的装饰（如此处的选中框），
 * 实际的图像是由子项 pixmapItem 自动绘制的，无需在此处手动调用。
 * @param painter 用于绘制的 QPainter 对象。
 * @param option 提供样式选项。
 * @param widget 绘制所在的窗口部件。
 */
void InteractivePixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // 如果该项当前被选中，则在其周围绘制一个虚线框作为视觉反馈。
    if (isSelected()) {
        painter->setPen(Qt::DashLine);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(pixmapItem->boundingRect());
    }
}

/**
 * @brief 鼠标按下事件处理器。
 *
 * 当用户点击该项时，发射一个信号通知外部（如场景或窗口），
 * 并调用基类实现以处理默认的点击行为（如选中和移动）。
 * @param event 鼠标场景事件。
 */
void InteractivePixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // 发射信号，将自身指针传递出去，以便外部可以识别是哪个项被点击了。
    emit itemClicked(this);
    // 调用基类的事件处理器，这是确保选中和移动功能正常工作的关键。
    QGraphicsObject::mousePressEvent(event);
}

/**
 * @brief 鼠标滚轮事件处理器。
 *
 * 实现通过滚轮缩放项的功能。
 * @param event 鼠标滚轮场景事件。
 */
void InteractivePixmapItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    // 只有当项被选中且没有按下Ctrl键时，才执行缩放。
    // (Ctrl+滚轮通常用于整个视图的缩放)。
    if (!isSelected() || event->modifiers() == Qt::ControlModifier) {
        event->ignore(); // 忽略事件，让其传递给父项或场景处理
        return;
    }

    // 根据滚轮滚动的方向确定缩放因子
    qreal scaleFactor = (event->delta() > 0) ? 1.1 : (1.0 / 1.1);
    // 将当前缩放值乘以缩放因子，并应用到项上
    setScale(scale() * scaleFactor);

    // 接受事件，表示我们已经处理了它，防止其进一步传递。
    event->accept();
}
