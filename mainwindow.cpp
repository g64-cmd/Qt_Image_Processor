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
#include "histogramwidget.h" // <--- 包含新头文件

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
    // 在构造函数的 ui->setupUi(this); 之后添加
    ui->imageSharpenButton->setIcon(QIcon(":/icons/resources/icons/edit-3.svg")); // 请替换为您下载的实际文件名
    ui->imageGrayscaleButton->setIcon(QIcon(":/icons/resources/icons/circle.svg"));
    ui->cannyButton->setIcon(QIcon(":/icons/resources/icons/crop.svg"));
    ui->imageStitchButton->setIcon(QIcon(":/icons/resources/icons/grid.svg"));
    ui->imageBlendButton->setIcon(QIcon(":/icons/resources/icons/layers.svg"));
    ui->textureMigrationButton->setIcon(QIcon(":/icons/resources/icons/image.svg"));
    ui->beautyButton->setIcon(QIcon(":/icons/resources/icons/smile.svg"));
    ui->gamma->setIcon(QIcon(":/icons/resources/icons/sun.svg"));

    // 调整图标大小，使其看起来更精致
    const QSize iconSize(20, 20); // 定义一个统一的图标尺寸
    ui->imageSharpenButton->setIconSize(iconSize);
    ui->imageGrayscaleButton->setIconSize(iconSize);
    ui->cannyButton->setIconSize(iconSize);
    ui->imageStitchButton->setIconSize(iconSize);
    ui->imageBlendButton->setIconSize(iconSize);
    ui->textureMigrationButton->setIconSize(iconSize);
    ui->beautyButton->setIconSize(iconSize);
    ui->gamma->setIconSize(iconSize);


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

    // --- 新增：连接颜色拾取器信号 ---
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

    // --- 伽马滑块初始化 ---
    ui->gammaSlider->setRange(10, 300);
    ui->gammaSlider->setValue(100);
    ui->gammaSlider->setEnabled(false);

    // --- 色彩调整滑块初始化 ---
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

    // --- 新增：初始化信息面板 ---
    ui->colorSwatchLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    ui->colorSwatchLabel->setAutoFillBackground(true);
    updateExtraInfoPanels(QPixmap()); // 初始状态清空
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

    resetAdjustmentSliders();
    ui->gammaSlider->setEnabled(true);
    ui->brightnessSlider->setEnabled(true);
    ui->contrastSlider->setEnabled(true);
    ui->saturationSlider->setEnabled(true);
    ui->hueSlider->setEnabled(true);

    processedPixmap = pixmap;
    currentSavePath.clear();
    updateDisplayImage(processedPixmap);
    fitToWindow();
    updateImageInfo();

    // --- 新增：更新直方图和信息面板 ---
    updateExtraInfoPanels(processedPixmap);

    ui->statusbar->showMessage(tr("已加载: %1").arg(currentStagedImageId), 3000);
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

    // --- 新增：更新直方图和信息面板 ---
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

    // --- 新增：更新直方图和信息面板 ---
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

// --- 新增槽函数实现 ---
void MainWindow::onMouseMovedOnImage(const QPointF &scenePos)
{
    if (processedPixmap.isNull() || !pixmapItem || !pixmapItem->sceneBoundingRect().contains(scenePos)) {
        // 鼠标不在图片上，清空信息
        ui->colorPosLabel->setText("Pos:");
        ui->colorRgbLabel->setText("RGB:");
        ui->colorHexLabel->setText("HEX:");
        QPalette palette = ui->colorSwatchLabel->palette();
        palette.setColor(QPalette::Window, Qt::lightGray);
        ui->colorSwatchLabel->setPalette(palette);
        return;
    }

    // 将场景坐标转换为图片内的像素坐标
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

// --- 新增辅助函数实现 ---
void MainWindow::updateExtraInfoPanels(const QPixmap &pixmap)
{
    ui->histogramWidget->updateHistogram(pixmap.toImage());

    // 如果图像为空，也清空颜色信息
    if (pixmap.isNull()) {
        onMouseMovedOnImage(QPointF(-1, -1)); // 传入一个无效点来清空信息
    }
}
