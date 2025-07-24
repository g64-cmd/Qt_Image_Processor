#ifndef DROPPABLEGRAPHICSVIEW_H
#define DROPPABLEGRAPHICSVIEW_H

#include <QGraphicsView>

class DroppableGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit DroppableGraphicsView(QWidget *parent = nullptr);

signals:
    void stagedImageDropped(const QString &imageId);
    void mouseMovedOnScene(const QPointF &scenePos);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // DROPPABLEGRAPHICSVIEW_H
