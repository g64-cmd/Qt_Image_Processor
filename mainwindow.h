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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// =============================================================================
// File: mainwindow.h
//
// Description:
// 该文件定义了应用程序的主窗口类 MainWindow。
// MainWindow 类是整个用户界面的核心，负责：
// - 构建和管理主窗口的UI元素。
// - 响应用户的交互，如菜单点击、按钮操作、快捷键等。
// - 协调各个功能模块，如图像处理器、视频处理器、暂存区管理器等。
// - 显示图像和视频内容，并更新相关信息。
// - 实现撤销/重做功能栈。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QMainWindow>
#include <QPixmap>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

// --- 前置声明 (Forward Declarations) ---
// 使用前置声明可以减少头文件之间的依赖，加快编译速度。
class QKeyEvent;
class QModelIndex;
class StagingAreaManager;
class DraggableItemModel;
class QUndoStack;
class ProcessCommand;
class HistogramWidget;
class VideoProcessor;


// Qt UI类的命名空间
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @class MainWindow
 * @brief 应用程序的主窗口类。
 *
 * 作为应用程序的中心控制器，管理UI交互和业务逻辑的分发。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

    // 将ProcessCommand声明为友元类。
    // 这允许ProcessCommand在执行和撤销操作时，直接调用MainWindow的私有更新函数
    // (如 updateImageFromCommand)，从而更新UI状态。这是命令模式的一种常见实现。
    friend class ProcessCommand;

public:
    /**
     * @brief 构造函数。
     * @param parent 父窗口部件，默认为nullptr。
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~MainWindow();

protected:
    // --- Qt事件处理 (Event Handling) ---

    /**
     * @brief 事件过滤器，用于捕获和处理子控件的特定事件。
     * 在此应用中，主要用于监听图像视图上的鼠标滚轮事件以实现缩放。
     * @param watched 被监视的对象。
     * @param event 发生的事件。
     * @return 如果事件被处理，则返回true；否则返回false。
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

    /**
     * @brief 键盘按下事件处理器，用于处理全局快捷键。
     * @param event 键盘事件对象。
     */
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // --- 文件菜单操作 (File Menu Actions) ---
    void on_actionopen_triggered();
    void on_actionsave_triggered();
    void on_actionsave_as_triggered();
    void on_actionexit_triggered();

    // --- 图像处理功能按钮 (Image Processing Buttons) ---
    void on_imageSharpenButton_clicked();
    void on_imageGrayscaleButton_clicked();
    void on_cannyButton_clicked();
    void on_imageStitchButton_clicked();
    void on_imageBlendButton_clicked();
    void on_textureMigrationButton_clicked();
    void on_beautyButton_clicked();
    void on_imageNewStitchButton_clicked();

    // --- 暂存区与历史记录 (Staging Area & History) ---
    void on_recentImageView_clicked(const QModelIndex &index);
    void onStagedImageDropped(const QString &imageId);
    void on_deleteStagedImageButton_clicked();

    // --- 图像色彩与色调调整 (Color & Tone Adjustments) ---
    void on_gamma_clicked();
    void on_gammaSlider_valueChanged(int value);
    void on_brightnessSlider_valueChanged(int value);
    void on_contrastSlider_valueChanged(int value);
    void on_saturationSlider_valueChanged(int value);
    void on_hueSlider_valueChanged(int value);
    void on_applyAdjustmentsButton_clicked();

    // --- 视频处理与播放 (Video Processing & Playback) ---
    void updateVideoFrame(const QPixmap &frame);
    void updateVideoProgress(const QString &timeString, int position, int duration);
    void onVideoOpened(bool success, int totalDurationMs, double fps);

    // --- 杂项UI交互 (Miscellaneous UI Interactions) ---
    void onMouseMovedOnImage(const QPointF &scenePos);

private:
    // --- 内部辅助方法 (Private Helper Methods) ---

    // 图像显示与控制
    void scaleImage(double newScale);
    void fitToWindow();
    void updateDisplayImage(const QPixmap &pixmap);
    void clearMainView();

    // 文件与数据管理
    void loadNewImageFromFile(const QString &filePath);
    void displayImageFromStagingArea(const QString &imageId);
    bool saveImageToFile(const QString &filePath);

    // 状态更新
    void updateImageInfo();
    void updateExtraInfoPanels(const QPixmap &pixmap);
    void resetAdjustmentSliders();

    // 命令模式相关
    void updateImageFromCommand(const QString &imageId, const QPixmap &pixmap);
    QString getCurrentImageId() const;
    QPixmap getCurrentImagePixmap() const;

    // 图像处理应用
    void applyAllAdjustments();

private:
    // --- 成员变量 (Member Variables) ---

    // UI相关
    Ui::MainWindow *ui;                 // Qt Designer生成的UI类实例
    QGraphicsScene *imageScene;         // 用于显示静态图像的场景
    QGraphicsPixmapItem *pixmapItem;    // 在场景中显示的图像项
    QGraphicsScene *videoScene;         // 用于显示视频帧的场景
    QGraphicsPixmapItem *videoPixmapItem; // 在场景中显示的视频帧项

    // 状态与数据
    QString currentStagedImageId;       // 当前在主视图中显示的图像在暂存区的ID
    QString currentSavePath;            // 当前文件的保存路径
    QString currentBaseName;            // 当前文件的基本名称（不含路径）
    double scaleFactor;                 // 当前图像的缩放因子
    QPixmap processedPixmap;            // 当前经过处理后显示的图像

    // 核心功能模块
    StagingAreaManager *stagingManager; // 管理暂存区图像的添加、删除和获取
    DraggableItemModel *stagingModel;   // 为暂存区视图提供数据模型
    QUndoStack *undoStack;              // 管理撤销/重做操作的栈
    VideoProcessor *videoProcessor;     // 负责视频文件的解码和处理

    // 实时调整参数
    int currentBrightness;              // 当前亮度滑块的值
    int currentContrast;                // 当前对比度滑块的值
    int currentSaturation;              // 当前饱和度滑块的值
    int currentHue;                     // 当前色相滑块的值
};

#endif // MAINWINDOW_H
