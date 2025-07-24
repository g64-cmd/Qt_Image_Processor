#ifndef STITCHERDIALOG_H
#define STITCHERDIALOG_H

#include <QDialog>
#include <QPixmap>

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
