// =============================================================================
//
// Copyright (C) 2025 g64-cmd
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// =============================================================================

// =============================================================================
// File: mainwindow.cpp
//
// Description:
// MainWindow类的实现文件。该文件包含了应用程序主窗口的所有业务逻辑，
// 包括UI初始化、事件处理、槽函数实现以及与各个功能模块的交互。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "mainwindow.h"
#include "ui_mainwindow.h"

// --- 包含自定义模块 ---
#include "beautydialog.h"
#include "draggableitemmodel.h"
#include "droppablegraphicsview.h"
#include "histogramwidget.h"
#include "imageblenddialog.h"
#include "imageconverter.h"
#include "imageprocessor.h"
#include "imagetexturetransferdialog.h"
#include "newstitcherdialog.h"
#include "processcommand.h"
#include "stagingareamanager.h"
#include "stitcherdialog.h"
#include "videoprocessor.h"

// --- 包含Qt模块 ---
#include <QCloseEvent>
#include <QFileDialog>
#include <QGuiApplication>
#include <QImageReader>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>
#include <QFileInfo>
#include <QStringListModel>
#include <QTimer>
#include <QUndoStack>
#include <QtMath>


// =============================================================================
// 构造函数与析构函数 (Constructor & Destructor)
// =============================================================================

/**
 * @brief MainWindow 构造函数。
 *
 * 负责初始化UI、创建和配置所有子系统（如图形视图、暂存区、
 * 撤销栈、视频处理器等），并建立它们之间的信号-槽连接。
 * @param parent 父窗口部件。
 */
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
    , videoProcessor(nullptr)
    , videoScene(nullptr)
    , videoPixmapItem(nullptr)
{
    // --- 1. UI基础设置 ---
    ui->setupUi(this);

    // --- 2. UI图标设置 ---
    const QSize iconSize(20, 20);
    // 为各个功能按钮设置图标和图标大小
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

    // --- 3. 主图像视图初始化 (QGraphicsView) ---
    imageScene = new QGraphicsScene(this);
    ui->graphicsView->setScene(imageScene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag); // 允许用手型光标拖动视图
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);      // 抗锯齿渲染
    ui->graphicsView->setRenderHint(QPainter::SmoothPixmapTransform); // 平滑的图像变换
    ui->graphicsView->setAlignment(Qt::AlignCenter);              // 图像居中对齐
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse); // 缩放以鼠标位置为中心
    ui->graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter); // 调整大小时以视图中心为锚点
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    ui->graphicsView->viewport()->installEventFilter(this); // 安装事件过滤器以捕获滚轮事件
    connect(ui->graphicsView, &DroppableGraphicsView::mouseMovedOnScene, this, &MainWindow::onMouseMovedOnImage);

    // --- 4. 暂存区设置 (Staging Area) ---
    stagingModel = new DraggableItemModel(this);
    stagingManager = new StagingAreaManager(stagingModel, this);
    ui->recentImageView->setModel(stagingModel);
    ui->recentImageView->setViewMode(QListView::IconMode); // 图标模式显示
    ui->recentImageView->setIconSize(QSize(100, 100));     // 设置缩略图大小
    ui->recentImageView->setResizeMode(QListView::Adjust); // 自动调整布局
    ui->recentImageView->setWordWrap(true);
    ui->recentImageView->setDragEnabled(true); // 允许拖拽
    connect(ui->recentImageView, &QListView::clicked, this, &MainWindow::on_recentImageView_clicked);
    connect(ui->graphicsView, &DroppableGraphicsView::stagedImageDropped, this, &MainWindow::onStagedImageDropped);

    // --- 5. 撤销/重做栈设置 (Undo/Redo Stack) ---
    undoStack = new QUndoStack(this);
    connect(ui->actionundo, &QAction::triggered, undoStack, &QUndoStack::undo);
    connect(ui->actionredo, &QAction::triggered, undoStack, &QUndoStack::redo);
    // 根据栈的状态自动启用/禁用撤销和重做按钮
    connect(undoStack, &QUndoStack::canUndoChanged, ui->actionundo, &QAction::setEnabled);
    connect(undoStack, &QUndoStack::canRedoChanged, ui->actionredo, &QAction::setEnabled);

    // --- 6. 色彩调整面板设置 (Color Adjustment Panel) ---
    ui->gammaSlider->setRange(10, 300);      // Gamma: 0.1 to 3.0
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

    // 设置颜色拾取器预览框的样式
    ui->colorSwatchLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    ui->colorSwatchLabel->setAutoFillBackground(true);

    // --- 7. 视频播放器模块设置 (Video Player Module) ---
    videoScene = new QGraphicsScene(this);
    ui->videoView->setScene(videoScene);
    videoProcessor = new VideoProcessor(ui, this); // 将ui指针传递给VideoProcessor
    connect(ui->addVideoButton, &QPushButton::clicked, videoProcessor, &VideoProcessor::addVideos);
    connect(ui->removeVideoButton, &QPushButton::clicked, videoProcessor, &VideoProcessor::removeSelectedVideo);
    connect(ui->videoListView, &QListView::clicked, videoProcessor, &VideoProcessor::playVideoAtIndex);
    connect(ui->playPauseButton, &QPushButton::clicked, videoProcessor, &VideoProcessor::togglePlayPause);
    connect(ui->videoSlider, &QSlider::sliderPressed, videoProcessor, &VideoProcessor::onSliderPressed);
    connect(ui->videoSlider, &QSlider::sliderMoved, videoProcessor, &VideoProcessor::seek);
    connect(ui->videoSlider, &QSlider::sliderReleased, videoProcessor, &VideoProcessor::stopSeeking);
    connect(ui->speedComboBox, &QComboBox::currentIndexChanged, videoProcessor, &VideoProcessor::setSpeed);
    connect(ui->saveFrameButton, &QPushButton::clicked, videoProcessor, &VideoProcessor::saveCurrentFrame);
    connect(ui->recordButton, &QPushButton::clicked, videoProcessor, &VideoProcessor::toggleRecording);

    // 连接VideoProcessor的信号到MainWindow的槽
    connect(videoProcessor, &VideoProcessor::frameReady, this, &MainWindow::updateVideoFrame);
    connect(videoProcessor, &VideoProcessor::progressUpdated, this, &MainWindow::updateVideoProgress);
    connect(videoProcessor, &VideoProcessor::videoOpened, this, &MainWindow::onVideoOpened);

    // --- 8. 初始化信息面板 ---
    updateExtraInfoPanels(QPixmap()); // 使用空Pixmap初始化直方图和颜色信息
}

/**
 * @brief MainWindow 析构函数。
 *
 * 清理Qt Designer生成的UI对象。其他QObject子对象会因父子关系被自动销毁。
 */
MainWindow::~MainWindow()
{
    delete ui;
}


// =============================================================================
// 事件处理 (Event Handling)
// =============================================================================

/**
 * @brief 事件过滤器，用于拦截和处理子控件的事件。
 *
 * 主要用于拦截 graphicsView 视口上的滚轮事件 (QEvent::Wheel)，
 * 以实现通过鼠标滚轮缩放图像的功能。
 * @param watched 被监视的对象。
 * @param event 发生的事件。
 * @return 如果事件被处理则返回 true，否则调用基类实现。
 */
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->graphicsView->viewport() && event->type() == QEvent::Wheel && pixmapItem) {
        auto *wheelEvent = static_cast<QWheelEvent*>(event);
        int angle = wheelEvent->angleDelta().y();
        double zoomFactor = (angle > 0) ? 1.15 : (1.0 / 1.15); // 向上滚动放大1.15倍，向下缩小
        scaleImage(scaleFactor * zoomFactor);
        return true; // 事件已处理，不再向下传递
    }
    return QMainWindow::eventFilter(watched, event);
}

/**
 * @brief 键盘按下事件处理器。
 *
 * 处理全局快捷键，如 Ctrl + '+' 放大，Ctrl + '-' 缩小。
 * @param event 键盘事件。
 */
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
        }
    }
    QMainWindow::keyPressEvent(event);
}


// =============================================================================
// 文件 I/O 操作 (File I/O Operations)
// =============================================================================

/**
 * @brief 槽函数：响应“打开”菜单动作。
 *
 * 弹出文件对话框让用户选择图像文件，并调用 `loadNewImageFromFile` 加载。
 */
void MainWindow::on_actionopen_triggered()
{
    // 支持常见的图像格式
    const QString filter = tr("Image Files (*.png *.jpg *.jpeg *.bmp);;All Files (*)");
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开图像"), "", filter);
    if (!fileName.isEmpty()) {
        loadNewImageFromFile(fileName);
    }
}

/**
 * @brief 槽函数：响应“保存”菜单动作。
 *
 * 如果文件已有保存路径，则直接保存。否则，行为同“另存为”。
 */
void MainWindow::on_actionsave_triggered()
{
    if (currentStagedImageId.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("当前没有可保存的图片。"));
        return;
    }

    if (currentSavePath.isEmpty()) {
        on_actionsave_as_triggered(); // 如果没有路径，则调用另存为
    } else {
        saveImageToFile(currentSavePath);
    }
}

/**
 * @brief 槽函数：响应“另存为”菜单动作。
 *
 * 弹出文件对话框让用户选择保存路径和格式，并调用 `saveImageToFile` 保存。
 */
void MainWindow::on_actionsave_as_triggered()
{
    if (currentStagedImageId.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("当前没有可保存的图片。"));
        return;
    }

    const QString filter = tr("PNG 文件 (*.png);;JPEG 文件 (*.jpg *.jpeg);;BMP 文件 (*.bmp)");
    QString fileName = QFileDialog::getSaveFileName(this, tr("另存为"), currentBaseName, filter);

    if (!fileName.isEmpty()) {
        if (saveImageToFile(fileName)) {
            currentSavePath = fileName; // 保存成功后，更新当前保存路径
        }
    }
}

/**
 * @brief 槽函数：响应“退出”菜单动作。
 */
void MainWindow::on_actionexit_triggered()
{
    close(); // 关闭主窗口，会触发QCloseEvent
}

// =============================================================================
// 图像处理槽函数 (Image Processing Slots)
// =============================================================================

/**
 * @brief 槽函数：锐化图像。
 *
 * 创建一个锐化命令并将其推入撤销栈。
 */
void MainWindow::on_imageSharpenButton_clicked()
{
    if (currentStagedImageId.isEmpty()) return;
    undoStack->push(new ProcessCommand(this, ProcessCommand::Sharpen));
}

/**
 * @brief 槽函数：灰度化图像。
 *
 * 创建一个灰度化命令并将其推入撤销栈。
 */
void MainWindow::on_imageGrayscaleButton_clicked()
{
    if (currentStagedImageId.isEmpty()) return;
    undoStack->push(new ProcessCommand(this, ProcessCommand::Grayscale));
}

/**
 * @brief 槽函数：Canny边缘检测。
 *
 * 创建一个Canny命令并将其推入撤销栈。
 */
void MainWindow::on_cannyButton_clicked()
{
    if (currentStagedImageId.isEmpty()) return;
    undoStack->push(new ProcessCommand(this, ProcessCommand::Canny));
}

/**
 * @brief 槽函数：打开旧版图像拼接对话框。
 */
void MainWindow::on_imageStitchButton_clicked()
{
    if (stagingManager->getImageCount() == 0) return;
    StitcherDialog dialog(stagingManager, stagingModel, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPixmap finalImage = dialog.getFinalImage();
        if (!finalImage.isNull()) {
            QString newId = stagingManager->addNewImage(finalImage, "stitched_image");
            if (!newId.isEmpty()) displayImageFromStagingArea(newId);
        }
    }
}

/**
 * @brief 槽函数：打开新版图像拼接对话框。
 */
void MainWindow::on_imageNewStitchButton_clicked()
{
    NewStitcherDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QPixmap finalImage = dialog.getResultImage();
        if (!finalImage.isNull()) {
            QString newId = stagingManager->addNewImage(finalImage, "new_stitched_image");
            if (!newId.isEmpty()) displayImageFromStagingArea(newId);
        }
    }
}

/**
 * @brief 槽函数：打开图像融合对话框。
 */
void MainWindow::on_imageBlendButton_clicked()
{
    if (currentStagedImageId.isEmpty()) return;
    ImageBlendDialog dialog(processedPixmap, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPixmap finalImage = dialog.getBlendedImage();
        if (!finalImage.isNull()) {
            QString newId = stagingManager->addNewImage(finalImage, "blended_image");
            if (!newId.isEmpty()) displayImageFromStagingArea(newId);
        }
    }
}

/**
 * @brief 槽函数：打开纹理迁移对话框。
 */
void MainWindow::on_textureMigrationButton_clicked()
{
    if (currentStagedImageId.isEmpty()) return;
    ImageTextureTransferDialog dialog(processedPixmap, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPixmap finalImage = dialog.getResultImage();
        if (!finalImage.isNull()) {
            QString newId = stagingManager->addNewImage(finalImage, "texture_transfer_result");
            if (!newId.isEmpty()) displayImageFromStagingArea(newId);
        }
    }
}

/**
 * @brief 槽函数：打开美颜对话框。
 */
void MainWindow::on_beautyButton_clicked()
{
    if (currentStagedImageId.isEmpty()) return;
    BeautyDialog dialog(processedPixmap, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPixmap finalImage = dialog.getResultImage();
        if (!finalImage.isNull()) {
            QString newId = stagingManager->addNewImage(finalImage, "beautified_image");
            if (!newId.isEmpty()) displayImageFromStagingArea(newId);
        }
    }
}


// =============================================================================
// 色彩调整槽函数 (Color Adjustment Slots)
// =============================================================================

/**
 * @brief 槽函数：重置Gamma滑块的值为默认值100 (1.0)。
 */
void MainWindow::on_gamma_clicked()
{
    if (!currentStagedImageId.isEmpty()) {
        ui->gammaSlider->setValue(100);
    }
}

/**
 * @brief 槽函数：当Gamma滑块值改变时，应用所有调整。
 */
void MainWindow::on_gammaSlider_valueChanged(int)
{
    applyAllAdjustments();
}

/**
 * @brief 槽函数：当亮度滑块值改变时，更新亮度值并应用所有调整。
 * @param value 新的亮度值。
 */
void MainWindow::on_brightnessSlider_valueChanged(int value)
{
    currentBrightness = value;
    applyAllAdjustments();
}

/**
 * @brief 槽函数：当对比度滑块值改变时，更新对比度值并应用所有调整。
 * @param value 新的对比度值。
 */
void MainWindow::on_contrastSlider_valueChanged(int value)
{
    currentContrast = value;
    applyAllAdjustments();
}

/**
 * @brief 槽函数：当饱和度滑块值改变时，更新饱和度值并应用所有调整。
 * @param value 新的饱和度值。
 */
void MainWindow::on_saturationSlider_valueChanged(int value)
{
    currentSaturation = value;
    applyAllAdjustments();
}

/**
 * @brief 槽函数：当色相滑块值改变时，更新色相值并应用所有调整。
 * @param value 新的色相值。
 */
void MainWindow::on_hueSlider_valueChanged(int value)
{
    currentHue = value;
    applyAllAdjustments();
}

/**
 * @brief 槽函数：将当前的色彩调整结果应用为一个新的暂存区图像。
 */
void MainWindow::on_applyAdjustmentsButton_clicked()
{
    if (currentStagedImageId.isEmpty() || processedPixmap.isNull()) {
        QMessageBox::information(this, "提示", "没有可应用的参数调整。");
        return;
    }

    // 创建一个不带 "_adjusted" 后缀的新名称
    QString baseName = stagingManager->getStagedImage(currentStagedImageId).name;
    baseName.remove(QRegularExpression("_adjusted_?\\d*$"));

    // 将调整后的图像作为新图像添加到暂存区
    QString newId = stagingManager->addNewImage(processedPixmap, baseName + "_adjusted");

    if (!newId.isEmpty()) {
        displayImageFromStagingArea(newId);
        // 选中新创建的图像
        QModelIndex index = stagingModel->index(0, 0);
        ui->recentImageView->setCurrentIndex(index);
        statusBar()->showMessage("参数调整已应用为新副本。", 3000);
    }
}


// =============================================================================
// 暂存区与UI交互 (Staging Area & UI Interaction)
// =============================================================================

/**
 * @brief 槽函数：当暂存区中的图像被点击时调用。
 * @param index 被点击项的模型索引。
 */
void MainWindow::on_recentImageView_clicked(const QModelIndex &index)
{
    QString imageId = index.data(Qt::UserRole).toString();
    if (!imageId.isEmpty()) {
        displayImageFromStagingArea(imageId);
        // 使用QTimer::singleShot确保在当前事件处理完成后再执行promoteImage，
        // 避免在模型操作期间发生意外的UI更新。
        QTimer::singleShot(0, this, [this, imageId]() {
            stagingManager->promoteImage(imageId);
        });
    }
}

/**
 * @brief 槽函数：当一个暂存区图像被拖拽到主视图时调用。
 * @param imageId 被拖拽图像的ID。
 */
void MainWindow::onStagedImageDropped(const QString &imageId)
{
    displayImageFromStagingArea(imageId);
    QTimer::singleShot(0, this, [this, imageId]() {
        stagingManager->promoteImage(imageId);
    });
}

/**
 * @brief 槽函数：删除在暂存区中选中的图像。
 */
void MainWindow::on_deleteStagedImageButton_clicked()
{
    QModelIndexList selectedIndexes = ui->recentImageView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, "提示", "请先在暂存区中选择要删除的图片。");
        return;
    }

    // 使用QSet避免重复删除同一个ID
    QSet<QString> idsToDelete;
    for (const QModelIndex &index : selectedIndexes) {
        idsToDelete.insert(index.data(Qt::UserRole).toString());
    }

    for (const QString &id : idsToDelete) {
        stagingManager->removeImage(id);
        // 如果删除的是当前正在显示的图像，则清空主视图
        if (id == currentStagedImageId) {
            clearMainView();
        }
    }
    statusBar()->showMessage("选中的图片已从暂存区删除。", 3000);
}

/**
 * @brief 槽函数：当鼠标在图像上移动时更新颜色信息。
 * @param scenePos 鼠标在场景中的坐标。
 */
void MainWindow::onMouseMovedOnImage(const QPointF &scenePos)
{
    // 检查图像是否存在且鼠标在图像范围内
    if (processedPixmap.isNull() || !pixmapItem || !pixmapItem->sceneBoundingRect().contains(scenePos)) {
        // 如果不在范围内，清空信息
        ui->colorPosLabel->setText("Pos:");
        ui->colorRgbLabel->setText("RGB:");
        ui->colorHexLabel->setText("HEX:");
        QPalette palette = ui->colorSwatchLabel->palette();
        palette.setColor(QPalette::Window, Qt::lightGray);
        ui->colorSwatchLabel->setPalette(palette);
        return;
    }

    // 将场景坐标转换为图像像素坐标
    QPointF pixmapPos = pixmapItem->mapFromScene(scenePos);
    int x = qRound(pixmapPos.x());
    int y = qRound(pixmapPos.y());

    if (x >= 0 && x < processedPixmap.width() && y >= 0 && y < processedPixmap.height()) {
        QColor color = processedPixmap.toImage().pixelColor(x, y);
        // 更新UI标签
        ui->colorPosLabel->setText(QString("Pos: (%1, %2)").arg(x).arg(y));
        ui->colorRgbLabel->setText(QString("RGB: (%1, %2, %3)").arg(color.red()).arg(color.green()).arg(color.blue()));
        ui->colorHexLabel->setText(QString("HEX: %1").arg(color.name(QColor::HexRgb)).toUpper());

        // 更新颜色预览框
        QPalette palette = ui->colorSwatchLabel->palette();
        palette.setColor(QPalette::Window, color);
        ui->colorSwatchLabel->setPalette(palette);
    }
}


// =============================================================================
// 视频处理槽函数 (Video Processing Slots)
// =============================================================================

/**
 * @brief 槽函数：更新视频播放视图的当前帧。
 * @param frame 要显示的视频帧。
 */
void MainWindow::updateVideoFrame(const QPixmap &frame)
{
    if (frame.isNull()) return;

    if (!videoPixmapItem) {
        videoPixmapItem = videoScene->addPixmap(frame);
    } else {
        videoPixmapItem->setPixmap(frame);
    }
    videoScene->setSceneRect(frame.rect());
    ui->videoView->fitInView(videoPixmapItem, Qt::KeepAspectRatio);
}

/**
 * @brief 槽函数：更新视频播放进度。
 * @param timeString 格式化的时间字符串 (e.g., "00:10 / 01:30")。
 * @param position 当前播放位置（毫秒）。
 * @param duration 视频总时长（毫秒）。
 */
void MainWindow::updateVideoProgress(const QString &timeString, int position, int duration)
{
    ui->timeLabel->setText(timeString);
    // 仅在用户没有拖动滑块时才更新滑块位置
    if (!ui->videoSlider->isSliderDown()) {
        // 临时阻塞信号，避免在程序设置值时触发valueChanged信号
        ui->videoSlider->blockSignals(true);
        ui->videoSlider->setRange(0, duration);
        ui->videoSlider->setValue(position);
        ui->videoSlider->blockSignals(false);
    }
}

/**
 * @brief 槽函数：当视频成功打开时调用。
 * @param success 是否成功打开。
 * @param totalDurationMs 视频总时长（毫秒）。
 * @param fps 视频帧率。
 */
void MainWindow::onVideoOpened(bool success, int totalDurationMs, double fps)
{
    Q_UNUSED(fps);
    if(success) {
        ui->videoSlider->setRange(0, totalDurationMs);
    }
}


// =============================================================================
// 私有辅助方法 (Private Helper Methods)
// =============================================================================

/**
 * @brief 从文件加载新图像。
 *
 * 加载图像文件，将其添加到暂存区，并显示在主视图中。
 * @param filePath 图像文件的完整路径。
 */
void MainWindow::loadNewImageFromFile(const QString &filePath)
{
    QPixmap pixmap;
    if (!pixmap.load(filePath)) {
        QMessageBox::critical(this, tr("错误"), tr("无法加载图像文件: %1").arg(filePath));
        return;
    }

    currentBaseName = QFileInfo(filePath).baseName(); // 获取不含扩展名的文件名
    currentSavePath = filePath; // 记录原始路径

    // 将新图像添加到暂存区
    QString newId = stagingManager->addNewImage(pixmap, currentBaseName);
    if (!newId.isEmpty()) {
        displayImageFromStagingArea(newId);
    }
}

/**
 * @brief 从暂存区显示图像。
 *
 * 这是切换主视图显示图像的核心函数。
 * @param imageId 要显示的图像在暂存区的ID。
 */
void MainWindow::displayImageFromStagingArea(const QString &imageId)
{
    StagingAreaManager::StagedImage stagedImage = stagingManager->getStagedImage(imageId);
    if (stagedImage.pixmap.isNull()) return;

    undoStack->clear(); // 切换图像时清空撤销栈
    currentStagedImageId = imageId;
    currentBaseName = stagedImage.name;
    processedPixmap = stagedImage.pixmap; // 将处理后的图像重置为暂存区的原始图像
    currentSavePath.clear(); // 清除保存路径，强制用户“另存为”

    resetAdjustmentSliders(); // 重置所有调整滑块
    // 启用滑块
    ui->gammaSlider->setEnabled(true);
    ui->brightnessSlider->setEnabled(true);
    ui->contrastSlider->setEnabled(true);
    ui->saturationSlider->setEnabled(true);
    ui->hueSlider->setEnabled(true);

    updateDisplayImage(processedPixmap);
    fitToWindow(); // 自动调整缩放以适应窗口
    updateImageInfo();
    updateExtraInfoPanels(processedPixmap); // 更新直方图等信息面板

    statusBar()->showMessage(tr("已加载: %1").arg(currentBaseName), 3000);
}

/**
 * @brief 将当前处理的图像保存到文件。
 * @param filePath 目标文件路径。
 * @return 如果保存成功返回 true，否则返回 false。
 */
bool MainWindow::saveImageToFile(const QString &filePath)
{
    if (processedPixmap.isNull()) return false;

    if (processedPixmap.save(filePath)) {
        statusBar()->showMessage(tr("图像已成功保存至 %1").arg(filePath), 5000);
        return true;
    } else {
        QMessageBox::critical(this, "错误", tr("无法保存图像至 %1").arg(filePath));
        return false;
    }
}

/**
 * @brief 更新主视图中显示的图像。
 * @param pixmap 要显示的 QPixmap 对象。
 */
void MainWindow::updateDisplayImage(const QPixmap &pixmap)
{
    if (pixmap.isNull()) return;
    imageScene->clear(); // 清空场景
    pixmapItem = imageScene->addPixmap(pixmap); // 添加新图像
    imageScene->setSceneRect(pixmap.rect()); // 更新场景大小
}

/**
 * @brief 清空主图像视图和相关状态。
 */
void MainWindow::clearMainView()
{
    imageScene->clear();
    pixmapItem = nullptr;
    processedPixmap = QPixmap();
    currentStagedImageId.clear();

    updateImageInfo();
    updateExtraInfoPanels(QPixmap());
    resetAdjustmentSliders();

    // 禁用滑块
    ui->gammaSlider->setEnabled(false);
    ui->brightnessSlider->setEnabled(false);
    ui->contrastSlider->setEnabled(false);
    ui->saturationSlider->setEnabled(false);
    ui->hueSlider->setEnabled(false);
}

/**
 * @brief 应用所有实时色彩调整。
 *
 * 从暂存区获取原始图像，然后依次应用Gamma、亮度、对比度等所有滑块的调整。
 */
void MainWindow::applyAllAdjustments()
{
    if (currentStagedImageId.isEmpty()) return;
    QPixmap originalStagedPixmap = stagingManager->getPixmap(currentStagedImageId);
    if (originalStagedPixmap.isNull()) return;

    QImage tempImage = originalStagedPixmap.toImage();

    // 应用Gamma校正
    double gamma = ui->gammaSlider->value() / 100.0;
    if (!qFuzzyCompare(gamma, 1.0)) { // 仅在gamma不为1时应用
        tempImage = ImageProcessor::applyGamma(tempImage, gamma);
    }

    // 应用其他色彩调整
    tempImage = ImageProcessor::adjustColor(tempImage, currentBrightness, currentContrast, currentSaturation, currentHue);

    processedPixmap = QPixmap::fromImage(tempImage);
    updateDisplayImage(processedPixmap);
    updateExtraInfoPanels(processedPixmap); // 调整后更新直方图
}

/**
 * @brief 缩放图像。
 * @param newScale 新的缩放因子。
 */
void MainWindow::scaleImage(double newScale)
{
    // 限制缩放范围在 10% 到 1000% 之间
    double newBoundedScale = qBound(0.1, newScale, 10.0);
    if (qFuzzyCompare(scaleFactor, newBoundedScale)) return;

    double factor = newBoundedScale / scaleFactor;
    ui->graphicsView->scale(factor, factor);
    scaleFactor = newBoundedScale;

    statusBar()->showMessage(QString("缩放比例: %1%").arg(int(scaleFactor * 100)));
}

/**
 * @brief 使图像适应窗口大小。
 */
void MainWindow::fitToWindow()
{
    if (!pixmapItem) return;
    ui->graphicsView->fitInView(imageScene->sceneRect(), Qt::KeepAspectRatio);
    // 更新缩放因子为fitInView后的实际值
    scaleFactor = ui->graphicsView->transform().m11();
}

/**
 * @brief 更新UI上的图像信息（名称、分辨率、大小）。
 */
void MainWindow::updateImageInfo()
{
    if (processedPixmap.isNull()) {
        ui->imageNameLabel->setText(tr("图片名称:"));
        ui->imageResolutionLabel->setText(tr("分辨率:"));
        ui->imageSizeLabel->setText(tr("大小:"));
        return;
    }
    ui->imageNameLabel->setText(tr("图片名称: %1").arg(currentBaseName));
    ui->imageResolutionLabel->setText(tr("分辨率: %1 x %2").arg(processedPixmap.width()).arg(processedPixmap.height()));
    // 注意：sizeInBytes() 只是一个估算值
    ui->imageSizeLabel->setText(tr("大小: %1 KB").arg(processedPixmap.toImage().sizeInBytes() / 1024));
}

/**
 * @brief 更新附加信息面板（如直方图）。
 * @param pixmap 用于生成信息的图像。
 */
void MainWindow::updateExtraInfoPanels(const QPixmap &pixmap)
{
    ui->histogramWidget->updateHistogram(pixmap.toImage());
    if (pixmap.isNull()) {
        // 如果图像为空，也需要清空颜色拾取器信息
        onMouseMovedOnImage(QPointF(-1, -1));
    }
}

/**
 * @brief 重置所有色彩调整滑块到默认值。
 */
void MainWindow::resetAdjustmentSliders()
{
    // 在程序设置滑块值时阻塞信号，防止触发不必要的applyAllAdjustments调用
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

    // 恢复信号
    ui->gammaSlider->blockSignals(false);
    ui->brightnessSlider->blockSignals(false);
    ui->contrastSlider->blockSignals(false);
    ui->saturationSlider->blockSignals(false);
    ui->hueSlider->blockSignals(false);
}


// =============================================================================
// 命令模式相关方法 (Command Pattern Methods)
// =============================================================================

/**
 * @brief 由ProcessCommand调用，用于在执行/撤销/重做后更新UI。
 * @param imageId 目标图像的ID。
 * @param pixmap 更新后的图像。
 */
void MainWindow::updateImageFromCommand(const QString &imageId, const QPixmap &pixmap)
{
    if (currentStagedImageId != imageId) {
        displayImageFromStagingArea(imageId);
    }
    processedPixmap = pixmap;
    updateDisplayImage(processedPixmap);
    stagingManager->updateImage(imageId, pixmap); // 更新暂存区中的缩略图
    currentSavePath.clear(); // 处理后需要另存为
    updateImageInfo();
    updateExtraInfoPanels(processedPixmap);
}

/**
 * @brief 获取当前正在处理的图像ID。
 * @return 当前图像ID。
 */
QString MainWindow::getCurrentImageId() const
{
    return currentStagedImageId;
}

/**
 * @brief 获取当前主视图中经过处理的图像。
 * @return 当前显示的QPixmap。
 */
QPixmap MainWindow::getCurrentImagePixmap() const
{
    return processedPixmap;
}
