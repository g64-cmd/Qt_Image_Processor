//mainwindow.cpp
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
#include "beautydialog.h"
#include "histogramwidget.h"
#include "newstitcherdialog.h"

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
    , currentBrightness(0)
    , currentContrast(0)
    , currentSaturation(0)
    , currentHue(0)
{
    ui->setupUi(this);

    const QSize iconSize(20, 20);

    ui->applyAdjustmentsButton->setIcon(QIcon(":/icons/resources/icons/check-square.svg"));
    ui->imageSharpenButton->setIcon(QIcon(":/icons/resources/icons/edit-3.svg"));
    ui->imageGrayscaleButton->setIcon(QIcon(":/icons/resources/icons/circle.svg"));
    ui->cannyButton->setIcon(QIcon(":/icons/resources/icons/crop.svg"));
    ui->imageStitchButton->setIcon(QIcon(":/icons/resources/icons/grid.svg"));
    ui->imageBlendButton->setIcon(QIcon(":/icons/resources/icons/layers.svg"));
    ui->textureMigrationButton->setIcon(QIcon(":/icons/resources/icons/image.svg"));
    ui->beautyButton->setIcon(QIcon(":/icons/resources/icons/smile.svg"));
    ui->gamma->setIcon(QIcon(":/icons/resources/icons/sun.svg"));
    ui->imageNewStitchButton->setIcon(QIcon(":/icons/resources/icons/layout.svg"));
    ui->deleteStagedImageButton->setIcon(QIcon(":/icons/resources/icons/trash-2.svg"));

    ui->applyAdjustmentsButton->setIconSize(iconSize);
    ui->imageSharpenButton->setIconSize(iconSize);
    ui->imageGrayscaleButton->setIconSize(iconSize);
    ui->cannyButton->setIconSize(iconSize);
    ui->imageStitchButton->setIconSize(iconSize);
    ui->imageBlendButton->setIconSize(iconSize);
    ui->textureMigrationButton->setIconSize(iconSize);
    ui->beautyButton->setIconSize(iconSize);
    ui->gamma->setIconSize(iconSize);
    ui->imageNewStitchButton->setIconSize(iconSize);
    ui->deleteStagedImageButton->setIconSize(iconSize);


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

    connect(ui->graphicsView, &DroppableGraphicsView::mouseMovedOnScene, this, &MainWindow::onMouseMovedOnImage);

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

    ui->gammaSlider->setRange(10, 300);
    ui->gammaSlider->setValue(100);
    ui->gammaSlider->setEnabled(false);

    ui->brightnessSlider->setRange(-100, 100);
    ui->brightnessSlider->setValue(0);
    ui->brightnessSlider->setEnabled(false);
    connect(ui->brightnessSlider, &QSlider::valueChanged, this, &MainWindow::on_brightnessSlider_valueChanged);

    ui->contrastSlider->setRange(-100, 100);
    ui->contrastSlider->setValue(0);
    ui->contrastSlider->setEnabled(false);
    connect(ui->contrastSlider, &QSlider::valueChanged, this, &MainWindow::on_contrastSlider_valueChanged);

    ui->saturationSlider->setRange(-100, 100);
    ui->saturationSlider->setValue(0);
    ui->saturationSlider->setEnabled(false);
    connect(ui->saturationSlider, &QSlider::valueChanged, this, &MainWindow::on_saturationSlider_valueChanged);

    ui->hueSlider->setRange(-180, 180);
    ui->hueSlider->setValue(0);
    ui->hueSlider->setEnabled(false);
    connect(ui->hueSlider, &QSlider::valueChanged, this, &MainWindow::on_hueSlider_valueChanged);

    ui->colorSwatchLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    ui->colorSwatchLabel->setAutoFillBackground(true);
    updateExtraInfoPanels(QPixmap());
}

MainWindow::~MainWindow()
{
    delete ui;
}

// --- 新增：为新按钮添加槽函数实现 ---
void MainWindow::on_applyAdjustmentsButton_clicked()
{
    if (currentStagedImageId.isEmpty() || processedPixmap.isNull()) {
        QMessageBox::information(this, "提示", "没有可应用的参数调整。");
        return;
    }

    // 将当前处理后的图像作为一个新副本添加到暂存区
    QString baseName = stagingManager->getStagedImage(currentStagedImageId).name;
    // Remove previous suffixes like "_adjusted" before adding a new one
    baseName.remove(QRegularExpression("_adjusted_?\\d*$"));
    QString newId = stagingManager->addNewImage(processedPixmap, baseName + "_adjusted");

    if (!newId.isEmpty()) {
        // 选中并显示这个新创建的副本
        displayImageFromStagingArea(newId);
        // 让新项目在列表中可见
        QModelIndex index = stagingModel->index(0, 0); // 新项目总是在第一个
        ui->recentImageView->setCurrentIndex(index);
        statusBar()->showMessage("参数调整已应用为新副本。", 3000);
    }
}

void MainWindow::on_deleteStagedImageButton_clicked()
{
    QModelIndexList selectedIndexes = ui->recentImageView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, "提示", "请先在暂存区中选择要删除的图片。");
        return;
    }

    // Collect all unique IDs to be deleted
    QSet<QString> idsToDelete;
    for (const QModelIndex &index : selectedIndexes) {
        idsToDelete.insert(index.data(Qt::UserRole).toString());
    }

    for (const QString &id : idsToDelete) {
        stagingManager->removeImage(id);
        // 如果删除的是当前正在显示的图片，则清空主视图
        if (id == currentStagedImageId) {
            clearMainView();
        }
    }
    statusBar()->showMessage("选中的图片已从暂存区删除。", 3000);
}

void MainWindow::clearMainView()
{
    imageScene->clear();
    pixmapItem = nullptr;
    processedPixmap = QPixmap();
    currentStagedImageId.clear();
    updateImageInfo();
    updateExtraInfoPanels(QPixmap());
    resetAdjustmentSliders();
    ui->gammaSlider->setEnabled(false);
    ui->brightnessSlider->setEnabled(false);
    ui->contrastSlider->setEnabled(false);
    ui->saturationSlider->setEnabled(false);
    ui->hueSlider->setEnabled(false);
}


void MainWindow::on_imageNewStitchButton_clicked()
{
    NewStitcherDialog dialog(this);
    int ret = dialog.exec();

    if (ret == QDialog::Accepted) {
        QPixmap stitchedPixmap = dialog.getResultImage();
        if (!stitchedPixmap.isNull()) {
            QString newId = stagingManager->addNewImage(stitchedPixmap, "stitched_result");
            if (!newId.isEmpty()) {
                displayImageFromStagingArea(newId);
                statusBar()->showMessage("图像拼接成功！");
            }
        } else {
            statusBar()->showMessage("图像拼接失败或被取消。");
        }
    }
}


void MainWindow::displayImageFromStagingArea(const QString &imageId)
{
    StagingAreaManager::StagedImage stagedImage = stagingManager->getStagedImage(imageId);
    if (stagedImage.pixmap.isNull()) return;

    undoStack->clear();
    currentStagedImageId = imageId;
    currentBaseName = stagedImage.name;

    resetAdjustmentSliders();
    ui->gammaSlider->setEnabled(true);
    ui->brightnessSlider->setEnabled(true);
    ui->contrastSlider->setEnabled(true);
    ui->saturationSlider->setEnabled(true);
    ui->hueSlider->setEnabled(true);

    processedPixmap = stagedImage.pixmap;
    currentSavePath.clear();
    updateDisplayImage(processedPixmap);
    fitToWindow();
    updateImageInfo();

    updateExtraInfoPanels(processedPixmap);

    ui->statusbar->showMessage(tr("已加载: %1").arg(currentBaseName), 3000);
}

void MainWindow::on_gamma_clicked()
{
    if (!currentStagedImageId.isEmpty()) {
        ui->gammaSlider->setValue(100);
    }
}

void MainWindow::on_gammaSlider_valueChanged(int /*value*/)
{
    applyAllAdjustments();
}

void MainWindow::on_brightnessSlider_valueChanged(int value)
{
    currentBrightness = value;
    applyAllAdjustments();
}

void MainWindow::on_contrastSlider_valueChanged(int value)
{
    currentContrast = value;
    applyAllAdjustments();
}

void MainWindow::on_saturationSlider_valueChanged(int value)
{
    currentSaturation = value;
    applyAllAdjustments();
}

void MainWindow::on_hueSlider_valueChanged(int value)
{
    currentHue = value;
    applyAllAdjustments();
}

void MainWindow::applyAllAdjustments()
{
    if (currentStagedImageId.isEmpty()) return;

    QPixmap originalStagedPixmap = stagingManager->getPixmap(currentStagedImageId);
    if (originalStagedPixmap.isNull()) return;

    QImage tempImage = originalStagedPixmap.toImage();

    double gamma = ui->gammaSlider->value() / 100.0;
    if (!qFuzzyCompare(gamma, 1.0)) {
        tempImage = ImageProcessor::applyGamma(tempImage, gamma);
    }

    tempImage = ImageProcessor::adjustColor(tempImage, currentBrightness, currentContrast, currentSaturation, currentHue);

    processedPixmap = QPixmap::fromImage(tempImage);
    updateDisplayImage(processedPixmap);
    updateExtraInfoPanels(processedPixmap);
}

void MainWindow::resetAdjustmentSliders()
{
    ui->gammaSlider->blockSignals(true);
    ui->brightnessSlider->blockSignals(true);
    ui->contrastSlider->blockSignals(true);
    ui->saturationSlider->blockSignals(true);
    ui->hueSlider->blockSignals(true);

    ui->gammaSlider->setValue(100);
    ui->brightnessSlider->setValue(0);
    ui->contrastSlider->setValue(0);
    ui->saturationSlider->setValue(0);
    ui->hueSlider->setValue(0);

    currentBrightness = 0;
    currentContrast = 0;
    currentSaturation = 0;
    currentHue = 0;

    ui->gammaSlider->blockSignals(false);
    ui->brightnessSlider->blockSignals(false);
    ui->contrastSlider->blockSignals(false);
    ui->saturationSlider->blockSignals(false);
    ui->hueSlider->blockSignals(false);
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

    updateExtraInfoPanels(processedPixmap);
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
        ui->imageNameLabel->setText("图片名称:");
        ui->imageResolutionLabel->setText("分辨率:");
        ui->imageSizeLabel->setText("大小:");
        return;
    }
    ui->imageNameLabel->setText(QString("图片名称: %1").arg(currentBaseName));
    ui->imageResolutionLabel->setText(QString("分辨率: %1 x %2").arg(processedPixmap.width()).arg(processedPixmap.height()));
    ui->imageSizeLabel->setText(QString("大小: %1 KB").arg(processedPixmap.toImage().sizeInBytes() / 1024));
}

void MainWindow::onMouseMovedOnImage(const QPointF &scenePos)
{
    if (processedPixmap.isNull() || !pixmapItem || !pixmapItem->sceneBoundingRect().contains(scenePos)) {
        ui->colorPosLabel->setText("Pos:");
        ui->colorRgbLabel->setText("RGB:");
        ui->colorHexLabel->setText("HEX:");
        QPalette palette = ui->colorSwatchLabel->palette();
        palette.setColor(QPalette::Window, Qt::lightGray);
        ui->colorSwatchLabel->setPalette(palette);
        return;
    }

    QPointF pixmapPos = pixmapItem->mapFromScene(scenePos);
    int x = qRound(pixmapPos.x());
    int y = qRound(pixmapPos.y());

    if (x >= 0 && x < processedPixmap.width() && y >= 0 && y < processedPixmap.height()) {
        QColor color = processedPixmap.toImage().pixelColor(x, y);

        ui->colorPosLabel->setText(QString("Pos: (%1, %2)").arg(x).arg(y));
        ui->colorRgbLabel->setText(QString("RGB: (%1, %2, %3)").arg(color.red()).arg(color.green()).arg(color.blue()));
        ui->colorHexLabel->setText(QString("HEX: %1").arg(color.name(QColor::HexRgb)).toUpper());

        QPalette palette = ui->colorSwatchLabel->palette();
        palette.setColor(QPalette::Window, color);
        ui->colorSwatchLabel->setPalette(palette);
    }
}

void MainWindow::updateExtraInfoPanels(const QPixmap &pixmap)
{
    ui->histogramWidget->updateHistogram(pixmap.toImage());

    if (pixmap.isNull()) {
        onMouseMovedOnImage(QPointF(-1, -1));
    }
}
