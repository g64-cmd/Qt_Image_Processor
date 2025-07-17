#include "droppablegraphicsview.h"
#include <QMimeData>

// --- 关键修复：在这里定义全局常量 ---
// 这为在 .h 文件中声明的 extern 常量提供了实体定义
const QString STAGED_IMAGE_MIME_TYPE = "application/x-staged-image-id";

DroppableGraphicsView::DroppableGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setAcceptDrops(true);
}

void DroppableGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    // 检查包裹是否含有我们自定义的MIME类型
    if (event->mimeData()->hasFormat(STAGED_IMAGE_MIME_TYPE)) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void DroppableGraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(STAGED_IMAGE_MIME_TYPE)) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void DroppableGraphicsView::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();

    if (mimeData->hasFormat(STAGED_IMAGE_MIME_TYPE)) {
        // 提取出图片ID
        QString imageId = QString::fromUtf8(mimeData->data(STAGED_IMAGE_MIME_TYPE));
        if (!imageId.isEmpty()) {
            // 发射带有ID的信号
            emit stagedImageDropped(imageId);
        }
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}
