#include "stitcherdialog.h"
#include "ui_stitcherdialog.h"
#include "stagingareamanager.h"
#include "draggableitemmodel.h"
#include "droppablegraphicsview.h"
#include "interactivepixmapitem.h"
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QListView>
#include <QKeyEvent>
#include <QPainter> // 新增

StitcherDialog::StitcherDialog(StagingAreaManager *manager, DraggableItemModel *model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StitcherDialog),
    stagingManager(manager),
    sourceModel(model),
    zCounter(0.0)
{
    ui->setupUi(this);
    setWindowTitle("图像拼接画布");

    canvasView = new DroppableGraphicsView(this);
    QVBoxLayout *canvasLayout = new QVBoxLayout(ui->canvasPlaceholder);
    canvasLayout->setContentsMargins(0, 0, 0, 0);
    canvasLayout->addWidget(canvasView);

    scene = new QGraphicsScene(this);
    canvasView->setScene(scene);
    scene->setSceneRect(-1000, -1000, 2000, 2000);

    ui->sourceImagesView->setModel(sourceModel);
    ui->sourceImagesView->setViewMode(QListView::IconMode);
    ui->sourceImagesView->setIconSize(QSize(100, 100));
    ui->sourceImagesView->setResizeMode(QListView::Adjust);
    ui->sourceImagesView->setWordWrap(true);
    ui->sourceImagesView->setDragEnabled(true);

    connect(canvasView, &DroppableGraphicsView::stagedImageDropped, this, &StitcherDialog::onStagedImageDropped);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &StitcherDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &StitcherDialog::reject);
}

StitcherDialog::~StitcherDialog()
{
    delete ui;
}

// --- 新增：获取最终图片的实现 ---
QPixmap StitcherDialog::getFinalImage() const
{
    // 1. 获取能完全包围所有场景中项目（图片）的最小矩形
    QRectF bounds = scene->itemsBoundingRect();
    if (bounds.isEmpty()) {
        return QPixmap(); // 如果画布为空，返回空图片
    }

    // 2. 创建一个和边界矩形一样大的、透明的QImage作为画布
    QImage image(bounds.size().toSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    // 3. 创建一个QPainter，让它在这张新Image上作画
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    // 4. 关键一步：命令场景将边界矩形内的所有内容，绘制到我们的Painter上
    scene->render(&painter, QRectF(), bounds);
    painter.end();

    // 5. 将绘制好的QImage转换为QPixmap并返回
    return QPixmap::fromImage(image);
}


void StitcherDialog::onStagedImageDropped(const QString &imageId)
{
    if (imageId.isEmpty()) return;

    QPixmap pixmap = stagingManager->getPixmap(imageId);
    if (pixmap.isNull()) return;

    InteractivePixmapItem *item = new InteractivePixmapItem(pixmap);
    scene->addItem(item);

    connect(item, &InteractivePixmapItem::itemClicked, this, &StitcherDialog::bringItemToFront);

    bringItemToFront(item);

    item->setPos(canvasView->mapToScene(canvasView->viewport()->rect().center()));
}

void StitcherDialog::bringItemToFront(InteractivePixmapItem *item)
{
    zCounter += 1.0;
    item->setZValue(zCounter);
}

void StitcherDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::ShiftModifier) {
        QList<QGraphicsItem*> selected = scene->selectedItems();
        if (selected.isEmpty()) {
            QDialog::keyPressEvent(event);
            return;
        }

        qreal rotationAngle = 5.0;

        if (event->key() == Qt::Key_A) {
            for (QGraphicsItem *item : selected) {
                item->setRotation(item->rotation() - rotationAngle);
            }
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_D) {
            for (QGraphicsItem *item : selected) {
                item->setRotation(item->rotation() + rotationAngle);
            }
            event->accept();
            return;
        }
    }

    QDialog::keyPressEvent(event);
}
