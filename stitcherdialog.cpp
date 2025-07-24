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
#include <QPainter>

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

QPixmap StitcherDialog::getFinalImage() const
{
    QRectF bounds = scene->itemsBoundingRect();
    if (bounds.isEmpty()) {
        return QPixmap();
    }
    QImage image(bounds.size().toSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    scene->render(&painter, QRectF(), bounds);
    painter.end();
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
