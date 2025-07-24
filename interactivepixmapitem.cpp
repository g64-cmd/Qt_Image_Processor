#include "interactivepixmapitem.h"
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QtMath>

InteractivePixmapItem::InteractivePixmapItem(const QPixmap &pixmap, QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    pixmapItem = new QGraphicsPixmapItem(pixmap, this);
    QSizeF originalSize = pixmap.size();
    setTransformOriginPoint(originalSize.width() / 2, originalSize.height() / 2);
}

QRectF InteractivePixmapItem::boundingRect() const
{
    return pixmapItem->boundingRect();
}

void InteractivePixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (isSelected()) {
        painter->setPen(Qt::DashLine);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(pixmapItem->boundingRect());
    }
}

void InteractivePixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit itemClicked(this);
    QGraphicsObject::mousePressEvent(event);
}

void InteractivePixmapItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (!isSelected() || event->modifiers() == Qt::ControlModifier) {
        event->ignore();
        return;
    }
    qreal scaleFactor = (event->delta() > 0) ? 1.1 : (1.0 / 1.1);
    setScale(scale() * scaleFactor);
    event->accept();
}
