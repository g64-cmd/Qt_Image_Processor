#ifndef INTERACTIVEPIXMAPITEM_H
#define INTERACTIVEPIXMAPITEM_H

#include <QGraphicsObject>
#include <QPixmap>

class QGraphicsPixmapItem;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;

/**
 * @brief 一个可交互的 QGraphicsPixmapItem
 *
 * 封装了移动、缩放和层次管理的功能。旋转由父窗口通过键盘控制。
 */
class InteractivePixmapItem : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit InteractivePixmapItem(const QPixmap &pixmap, QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

signals:
    void itemClicked(InteractivePixmapItem *item);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void wheelEvent(QGraphicsSceneWheelEvent *event) override;

private:
    QGraphicsPixmapItem *pixmapItem;
};

#endif // INTERACTIVEPIXMAPITEM_H
