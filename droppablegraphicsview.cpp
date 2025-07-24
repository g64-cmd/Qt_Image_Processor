#include "droppablegraphicsview.h"
#include <QDragEnterEvent>
#include <QMimeData>
#include <QMouseEvent>

DroppableGraphicsView::DroppableGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setAcceptDrops(true);
    setMouseTracking(true);
}

void DroppableGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-draggable-item")) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void DroppableGraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-draggable-item")) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void DroppableGraphicsView::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasFormat("application/x-draggable-item")) {
        QByteArray itemData = mimeData->data("application/x-draggable-item");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        QString imageId;
        dataStream >> imageId;
        emit stagedImageDropped(imageId);
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void DroppableGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    emit mouseMovedOnScene(mapToScene(event->pos()));
    QGraphicsView::mouseMoveEvent(event);
}
