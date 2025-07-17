#ifndef DROPPABLEGRAPHICSVIEW_H
#define DROPPABLEGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QDragEnterEvent>
#include <QDropEvent>

// 定义外部可访问的MIME类型常量
extern const QString STAGED_IMAGE_MIME_TYPE;

class DroppableGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit DroppableGraphicsView(QWidget *parent = nullptr);

signals:
    /**
     * @brief 当一个暂存图片被成功拖放到视图上时，发射此信号
     * @param imageId 被拖放图片的唯一ID
     */
    void stagedImageDropped(const QString &imageId);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

#endif // DROPPABLEGRAPHICSVIEW_H
