#ifndef STITCHERDIALOG_H
#define STITCHERDIALOG_H

#include <QDialog>
#include <QPixmap> // 新增

// 向前声明
namespace Ui {
class StitcherDialog;
}
class StagingAreaManager;
class DraggableItemModel;
class QGraphicsScene;
class DroppableGraphicsView;
class InteractivePixmapItem;
class QKeyEvent;

class StitcherDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StitcherDialog(StagingAreaManager *manager, DraggableItemModel *model, QWidget *parent = nullptr);
    ~StitcherDialog();

    // 新增：公共函数，用于获取最终拼接好的图片
    QPixmap getFinalImage() const;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onStagedImageDropped(const QString &imageId);
    void bringItemToFront(InteractivePixmapItem *item);

private:
    Ui::StitcherDialog *ui;
    StagingAreaManager *stagingManager;
    DraggableItemModel *sourceModel;
    QGraphicsScene *scene;
    DroppableGraphicsView *canvasView;
    qreal zCounter = 0;
};

#endif // STITCHERDIALOG_H
