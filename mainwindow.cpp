#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "imageprocessor.h"
#include "stagingareamanager.h"
#include "draggableitemmodel.h"
#include "droppablegraphicsview.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QImageReader>
#include <QWheelEvent>
#include <QGuiApplication>
#include <QFileInfo>
#include <QtMath>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scaleFactor(1.0)
    , imageScene(nullptr)
    , pixmapItem(nullptr)
    , stagingManager(nullptr)
    , stagingModel(nullptr)
{
    ui->setupUi(this);

    imageScene = new QGraphicsScene(this);
    ui->graphicsView->setScene(imageScene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);
    ui->graphicsView->setAlignment(Qt::AlignCenter);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    ui->graphicsView->viewport()->installEventFilter(this);

    connect(ui->imageSharpenButton, &QPushButton::clicked, this, &MainWindow::on_imageSharpenButton_clicked);
    connect(ui->imageGrayscaleButton, &QPushButton::clicked, this, &MainWindow::on_imageGrayscaleButton_clicked);
    connect(ui->cannyButton, &QPushButton::clicked, this, &MainWindow::on_cannyButton_clicked);

    stagingModel = new DraggableItemModel(this);
    stagingManager = new StagingAreaManager(stagingModel, this);
    ui->recentImageView->setModel(stagingModel);
    ui->recentImageView->setViewMode(QListView::IconMode);
    ui->recentImageView->setIconSize(QSize(100, 100));
    ui->recentImageView->setResizeMode(QListView::Adjust);
    ui->recentImageView->setWordWrap(true);
    ui->recentImageView->setDragEnabled(true);
    connect(ui->recentImageView, &QListView::clicked, this, &MainWindow::on_recentImageView_clicked);

    connect(ui->graphicsView, &DroppableGraphicsView::stagedImageDropped, this, &MainWindow::onStagedImageDropped);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionopen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开图像"), "", "Image Files (*.png *.jpg *.bmp)");
    if (!fileName.isEmpty()) {
        loadNewImageFromFile(fileName);
    }
}

void MainWindow::loadNewImageFromFile(const QString &filePath)
{
    QPixmap pixmap;
    if (!pixmap.load(filePath)) {
        QMessageBox::critical(this, tr("错误"), tr("无法加载图像文件: %1").arg(filePath));
        return;
    }
    currentBaseName = QFileInfo(filePath).baseName();

    QString newId = stagingManager->addNewImage(pixmap, currentBaseName);
    if (!newId.isEmpty()) {
        displayImageFromStagingArea(newId);
    }
}

void MainWindow::displayImageFromStagingArea(const QString &imageId)
{
    QPixmap pixmap = stagingManager->getPixmap(imageId);
    if (pixmap.isNull()) return;

    currentStagedImageId = imageId;
    processedPixmap = pixmap;

    updateDisplayImage(processedPixmap);
    fitToWindow();
    updateImageInfo();

    ui->statusbar->showMessage(tr("已加载: %1").arg(currentStagedImageId), 3000);
}

void MainWindow::onStagedImageDropped(const QString &imageId)
{
    displayImageFromStagingArea(imageId);
    stagingManager->promoteImage(imageId);
}

void MainWindow::on_recentImageView_clicked(const QModelIndex &index)
{
    QString imageId = index.data(Qt::UserRole).toString();
    if (!imageId.isEmpty()) {
        displayImageFromStagingArea(imageId);
        stagingManager->promoteImage(imageId);
    }
}

void MainWindow::on_imageSharpenButton_clicked()
{
    if (currentStagedImageId.isEmpty() || processedPixmap.isNull()) {
        QMessageBox::information(this, "提示", "请先从暂存区选择一张图片。");
        return;
    }
    QImage resultImage = ImageProcessor::sharpen(processedPixmap.toImage());
    if (resultImage.isNull()) return;

    processedPixmap = QPixmap::fromImage(resultImage);
    updateDisplayImage(processedPixmap);
    stagingManager->updateImage(currentStagedImageId, processedPixmap);
    ui->statusbar->showMessage("图像锐化完成", 3000);
}

void MainWindow::on_imageGrayscaleButton_clicked()
{
    if (currentStagedImageId.isEmpty() || processedPixmap.isNull()) {
        QMessageBox::information(this, "提示", "请先从暂存区选择一张图片。");
        return;
    }
    QImage resultImage = ImageProcessor::grayscale(processedPixmap.toImage());
    if (resultImage.isNull()) return;

    processedPixmap = QPixmap::fromImage(resultImage);
    updateDisplayImage(processedPixmap);
    stagingManager->updateImage(currentStagedImageId, processedPixmap);
    ui->statusbar->showMessage("图像灰度化完成", 3000);
}

void MainWindow::on_cannyButton_clicked()
{
    if (currentStagedImageId.isEmpty() || processedPixmap.isNull()) {
        QMessageBox::information(this, "提示", "请先从暂存区选择一张图片。");
        return;
    }
    QImage resultImage = ImageProcessor::canny(processedPixmap.toImage());
    if (resultImage.isNull()) return;

    processedPixmap = QPixmap::fromImage(resultImage);
    updateDisplayImage(processedPixmap);
    stagingManager->updateImage(currentStagedImageId, processedPixmap);
    ui->statusbar->showMessage("Canny 边缘检测完成", 3000);
}

void MainWindow::updateDisplayImage(const QPixmap &pixmap)
{
    if (pixmap.isNull()) return;
    imageScene->clear();
    pixmapItem = imageScene->addPixmap(pixmap);
    imageScene->setSceneRect(pixmap.rect());
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->graphicsView->viewport() && event->type() == QEvent::Wheel && pixmapItem) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        int angle = wheelEvent->angleDelta().y();
        double zoomFactor = (angle > 0) ? 1.15 : (1.0 / 1.15);
        scaleImage(scaleFactor * zoomFactor);
        return true;
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (!pixmapItem) {
        QMainWindow::keyPressEvent(event);
        return;
    }
    if (event->modifiers() == Qt::ControlModifier) {
        if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
            scaleImage(scaleFactor * 1.2);
        } else if (event->key() == Qt::Key_Minus) {
            scaleImage(scaleFactor * 0.8);
        } else {
            QMainWindow::keyPressEvent(event);
        }
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::scaleImage(double newScale)
{
    double newBoundedScale = qBound(0.1, newScale, 10.0);
    if (qFuzzyCompare(scaleFactor, newBoundedScale)) {
        return;
    }
    double factor = newBoundedScale / scaleFactor;
    ui->graphicsView->scale(factor, factor);
    scaleFactor = newBoundedScale;
    ui->statusbar->showMessage(QString("缩放比例: %1%").arg(int(scaleFactor * 100)));
}

void MainWindow::fitToWindow()
{
    if (!pixmapItem) return;
    ui->graphicsView->fitInView(imageScene->sceneRect(), Qt::KeepAspectRatio);
    scaleFactor = ui->graphicsView->transform().m11();
}

void MainWindow::updateImageInfo()
{
    if (processedPixmap.isNull()) {
        ui->imageName->setText("图片名称");
        ui->imageFormat->setText("图片格式");
        ui->imageResolution->setText("图片分辨率");
        ui->imageSize->setText("图片大小");
        return;
    }

    ui->imageName->setText(currentBaseName);
    // --- 关键修复：将 hasAlpha() 替换为 hasAlphaChannel() ---
    ui->imageFormat->setText(processedPixmap.toImage().hasAlphaChannel() ? "PNG" : "JPG/BMP");
    ui->imageResolution->setText(QString("%1 x %2").arg(processedPixmap.width()).arg(processedPixmap.height()));
    ui->imageSize->setText(QString("%1 KB").arg(processedPixmap.toImage().sizeInBytes() / 1024));
}
