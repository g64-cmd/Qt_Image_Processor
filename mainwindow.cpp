#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "imageprocessor.h"
#include "stagingareamanager.h"
#include "draggableitemmodel.h"
#include "droppablegraphicsview.h"
#include "processcommand.h"
#include "stitcherdialog.h"
#include "imageblenddialog.h"
#include "imagetexturetransferdialog.h"
#include "beautydialog.h" // <-- 关键修复：包含美颜对话框的头文件

#include <QFileDialog>
#include <QMessageBox>
#include <QImageReader>
#include <QWheelEvent>
#include <QGuiApplication>
#include <QFileInfo>
#include <QtMath>
#include <QPainter>
#include <QCloseEvent>
#include <QUndoStack>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scaleFactor(1.0)
    , imageScene(nullptr)
    , pixmapItem(nullptr)
    , stagingManager(nullptr)
    , stagingModel(nullptr)
    , undoStack(nullptr)
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

    undoStack = new QUndoStack(this);
    connect(ui->actionundo, &QAction::triggered, undoStack, &QUndoStack::undo);
    connect(ui->actionredo, &QAction::triggered, undoStack, &QUndoStack::redo);
    connect(undoStack, &QUndoStack::canUndoChanged, ui->actionundo, &QAction::setEnabled);
    connect(undoStack, &QUndoStack::canRedoChanged, ui->actionredo, &QAction::setEnabled);

    // --- 伽马滑块初始化 ---
    ui->gammaSlider->setRange(10, 300); // 代表伽马值 0.1 到 3.0
    ui->gammaSlider->setValue(100);     // 默认值 1.0 (无变化)
    ui->gammaSlider->setEnabled(false); // 初始时禁用
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::displayImageFromStagingArea(const QString &imageId)
{
    QPixmap pixmap = stagingManager->getPixmap(imageId);
    if (pixmap.isNull()) return;
    undoStack->clear();
    currentStagedImageId = imageId;
    processedPixmap = pixmap;
    currentSavePath.clear();
    updateDisplayImage(processedPixmap);
    fitToWindow();
    updateImageInfo();

    // 加载新图片时，重置并启用滑块
    ui->gammaSlider->setValue(100);
    ui->gammaSlider->setEnabled(true);

    ui->statusbar->showMessage(tr("已加载: %1").arg(currentStagedImageId), 3000);
}

// --- 伽马变换的槽函数实现 ---
void MainWindow::on_gamma_clicked()
{
    // 点击按钮，将滑块和图像效果重置为默认值
    if (!currentStagedImageId.isEmpty()) {
        ui->gammaSlider->setValue(100);
    }
}

void MainWindow::on_gammaSlider_valueChanged(int value)
{
    if (currentStagedImageId.isEmpty()) {
        return; // 如果没有图片，则不执行任何操作
    }

    // 将滑块的整数值转换为浮点伽马值
    double gamma = value / 100.0;

    // 关键：为了避免效果叠加，我们总是从暂存区的原始图片开始处理
    QPixmap originalStagedPixmap = stagingManager->getPixmap(currentStagedImageId);
    if (originalStagedPixmap.isNull()) return;

    QImage resultImage = ImageProcessor::applyGamma(originalStagedPixmap.toImage(), gamma);
    if (!resultImage.isNull()) {
        // 更新当前画布上的图片
        processedPixmap = QPixmap::fromImage(resultImage);
        updateDisplayImage(processedPixmap);
        // 注意：伽马调整是一个“预览”步骤，我们不在这里更新暂存区或命令栈。
        // 当用户在此基础上再进行锐化等操作时，这个调整后的结果
        // 才会作为“操作前”的状态被存入命令。
    }
}

void MainWindow::on_imageStitchButton_clicked()
{
    if (stagingManager->getImageCount() == 0) {
        QMessageBox::information(this, "提示", "暂存区中没有图片可用于拼接。");
        return;
    }
    StitcherDialog dialog(stagingManager, stagingModel, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPixmap finalImage = dialog.getFinalImage();
        if (!finalImage.isNull()) {
            QString newId = stagingManager->addNewImage(finalImage, "stitched_image");
            if (!newId.isEmpty()) {
                displayImageFromStagingArea(newId);
            }
        }
    }
}

void MainWindow::on_imageBlendButton_clicked()
{
    if (currentStagedImageId.isEmpty() || processedPixmap.isNull()) {
        QMessageBox::information(this, "提示", "请先在主窗口中打开一张图片作为图层A。");
        return;
    }
    ImageBlendDialog dialog(processedPixmap, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPixmap finalImage = dialog.getBlendedImage();
        if (!finalImage.isNull()) {
            QString newId = stagingManager->addNewImage(finalImage, "blended_image");
            if (!newId.isEmpty()) {
                displayImageFromStagingArea(newId);
            }
        }
    }
}

void MainWindow::on_textureMigrationButton_clicked()
{
    if (currentStagedImageId.isEmpty() || processedPixmap.isNull()) {
        QMessageBox::information(this, "提示", "请先在主窗口中打开一张内容图。");
        return;
    }
    ImageTextureTransferDialog dialog(processedPixmap, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPixmap finalImage = dialog.getResultImage();
        if (!finalImage.isNull()) {
            QString newId = stagingManager->addNewImage(finalImage, "texture_transfer_result");
            if (!newId.isEmpty()) {
                displayImageFromStagingArea(newId);
            }
        }
    }
}

void MainWindow::on_beautyButton_clicked()
{
    if (currentStagedImageId.isEmpty() || processedPixmap.isNull()) {
        QMessageBox::information(this, "提示", "请先打开一张带有人脸的图片。");
        return;
    }
    BeautyDialog dialog(processedPixmap, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPixmap finalImage = dialog.getResultImage();
        if (!finalImage.isNull()) {
            QString newId = stagingManager->addNewImage(finalImage, "beautified_image");
            if (!newId.isEmpty()) {
                displayImageFromStagingArea(newId);
            }
        }
    }
}

void MainWindow::on_imageSharpenButton_clicked()
{
    if (currentStagedImageId.isEmpty()) {
        QMessageBox::information(this, "提示", "请先从暂存区选择一张图片。");
        return;
    }
    undoStack->push(new ProcessCommand(this, ProcessCommand::Sharpen));
}

void MainWindow::on_imageGrayscaleButton_clicked()
{
    if (currentStagedImageId.isEmpty()) {
        QMessageBox::information(this, "提示", "请先从暂存区选择一张图片。");
        return;
    }
    undoStack->push(new ProcessCommand(this, ProcessCommand::Grayscale));
}

void MainWindow::on_cannyButton_clicked()
{
    if (currentStagedImageId.isEmpty()) {
        QMessageBox::information(this, "提示", "请先从暂存区选择一张图片。");
        return;
    }
    undoStack->push(new ProcessCommand(this, ProcessCommand::Canny));
}

void MainWindow::updateImageFromCommand(const QString &imageId, const QPixmap &pixmap)
{
    if (currentStagedImageId != imageId) {
        displayImageFromStagingArea(imageId);
    }
    processedPixmap = pixmap;
    updateDisplayImage(processedPixmap);
    stagingManager->updateImage(imageId, processedPixmap);
    currentSavePath.clear();
    updateImageInfo();
}

QString MainWindow::getCurrentImageId() const
{
    return currentStagedImageId;
}

QPixmap MainWindow::getCurrentImagePixmap() const
{
    return processedPixmap;
}

void MainWindow::on_actionopen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开图像"), "", "Image Files (*.png *.jpg *.bmp)");
    if (!fileName.isEmpty()) {
        loadNewImageFromFile(fileName);
    }
}

void MainWindow::on_actionsave_triggered()
{
    if (currentStagedImageId.isEmpty()) {
        QMessageBox::information(this, "提示", "当前没有可保存的图片。");
        return;
    }
    if (currentSavePath.isEmpty()) {
        on_actionsave_as_triggered();
    } else {
        saveImageToFile(currentSavePath);
    }
}

void MainWindow::on_actionsave_as_triggered()
{
    if (currentStagedImageId.isEmpty()) {
        QMessageBox::information(this, "提示", "当前没有可保存的图片。");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this, "另存为", currentBaseName, "PNG 文件 (*.png);;JPEG 文件 (*.jpg *.jpeg)");
    if (!fileName.isEmpty()) {
        if (saveImageToFile(fileName)) {
            currentSavePath = fileName;
        }
    }
}

void MainWindow::on_actionexit_triggered()
{
    this->close();
}

bool MainWindow::saveImageToFile(const QString &filePath)
{
    if (processedPixmap.isNull()) return false;
    if (processedPixmap.save(filePath)) {
        ui->statusbar->showMessage(tr("图像已成功保存至 %1").arg(filePath), 5000);
        return true;
    } else {
        QMessageBox::critical(this, "错误", tr("无法保存图像至 %1").arg(filePath));
        return false;
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
    currentSavePath = filePath;
    QString newId = stagingManager->addNewImage(pixmap, currentBaseName);
    if (!newId.isEmpty()) {
        displayImageFromStagingArea(newId);
    }
}

void MainWindow::onStagedImageDropped(const QString &imageId)
{
    displayImageFromStagingArea(imageId);
    QTimer::singleShot(0, this, [this, imageId]() {
        stagingManager->promoteImage(imageId);
    });
}

void MainWindow::on_recentImageView_clicked(const QModelIndex &index)
{
    QString imageId = index.data(Qt::UserRole).toString();
    if (!imageId.isEmpty()) {
        displayImageFromStagingArea(imageId);
        QTimer::singleShot(0, this, [this, imageId]() {
            stagingManager->promoteImage(imageId);
        });
    }
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
    ui->imageFormat->setText(processedPixmap.toImage().hasAlphaChannel() ? "PNG" : "JPG/BMP");
    ui->imageResolution->setText(QString("%1 x %2").arg(processedPixmap.width()).arg(processedPixmap.height()));
    ui->imageSize->setText(QString("%1 KB").arg(processedPixmap.toImage().sizeInBytes() / 1024));
}
