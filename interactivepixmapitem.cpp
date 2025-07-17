#include "interactivepixmapitem.h"
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QtMath>

InteractivePixmapItem::InteractivePixmapItem(const QPixmap &pixmap, QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    // 允许此项被选中、移动，并能接收滚轮事件
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);

    pixmapItem = new QGraphicsPixmapItem(pixmap, this);
    QSizeF originalSize = pixmap.size();

    // 将所有变换（缩放、旋转）的原点设置为图片的中心
    setTransformOriginPoint(originalSize.width() / 2, originalSize.height() / 2);
}

QRectF InteractivePixmapItem::boundingRect() const
{
    // 边界矩形就是图片本身的矩形
    return pixmapItem->boundingRect();
}

void InteractivePixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // 如果此项被选中，只绘制一个简单的虚线边框
    if (isSelected()) {
        painter->setPen(Qt::DashLine);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(pixmapItem->boundingRect());
    }
}

void InteractivePixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // 发射信号，并调用基类方法以确保移动功能正常
    emit itemClicked(this);
    QGraphicsObject::mousePressEvent(event);
}

void InteractivePixmapItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    // 如果按下了Ctrl键，或者此项未被选中，则忽略事件，
    // 让它传递给 QGraphicsView 去处理画布的缩放。
    if (!isSelected() || event->modifiers() == Qt::ControlModifier) {
        event->ignore();
        return;
    }

    // 根据滚轮方向计算缩放因子
    qreal scaleFactor = (event->delta() > 0) ? 1.1 : (1.0 / 1.1);

    // 应用缩放
    setScale(scale() * scaleFactor);

    // 接受事件，防止它被继续传递
    event->accept();
}
