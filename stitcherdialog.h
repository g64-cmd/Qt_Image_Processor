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

#ifndef STITCHERDIALOG_H
#define STITCHERDIALOG_H

// =============================================================================
// File: stitcherdialog.h
//
// Description:
// 该文件定义了 StitcherDialog 类，这是一个用于手动、交互式图像拼接的
// 对话框。用户可以将暂存区的图像拖入一个画布，并手动调整它们的位置、
// 缩放和层叠顺序，以创建拼接图像。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QDialog>
#include <QPixmap>

// --- 前置声明 ---
namespace Ui {
class StitcherDialog;
}
class StagingAreaManager;
class DraggableItemModel;
class QGraphicsScene;
class DroppableGraphicsView;
class InteractivePixmapItem;
class QKeyEvent;

/**
 * @class StitcherDialog
 * @brief 提供手动图像拼接功能的对话框。
 *
 * 该对话框包含一个用于显示暂存区图像的列表和一个用于自由排列
 * 图像的画布（QGraphicsView）。用户可以通过拖放和键盘操作来
 * 控制画布上的图像。
 */
class StitcherDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     * @param manager 指向 StagingAreaManager 的指针，用于获取图像数据。
     * @param model 指向暂存区的数据模型，用于在对话框中显示。
     * @param parent 父窗口部件。
     */
    explicit StitcherDialog(StagingAreaManager *manager, DraggableItemModel *model, QWidget *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~StitcherDialog();

    /**
     * @brief 获取最终手动拼接的图像。
     *
     * 该函数会将当前 QGraphicsScene 的内容渲染成一张 QPixmap。
     * @return 返回最终的拼接结果图像。
     */
    QPixmap getFinalImage() const;

protected:
    /**
     * @brief 键盘按下事件处理器。
     *
     * 用于处理对选中图像项的旋转、删除等键盘操作。
     * @param event 键盘事件。
     */
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    /**
     * @brief 槽函数：当一个暂存区图像被拖放到画布上时调用。
     * @param imageId 被拖放的图像的唯一ID。
     */
    void onStagedImageDropped(const QString &imageId);

    /**
     * @brief 槽函数：将被点击的图像项带到最顶层显示。
     * @param item 被点击的 InteractivePixmapItem 指针。
     */
    void bringItemToFront(InteractivePixmapItem *item);

private:
    // --- 成员变量 ---
    Ui::StitcherDialog *ui;             // Qt Designer生成的UI类实例
    StagingAreaManager *stagingManager; // 用于访问暂存区图像数据的管理器
    DraggableItemModel *sourceModel;    // 暂存区的数据模型，用于在对话框的列表中显示
    QGraphicsScene *scene;              // 用于管理和显示可交互图像项的场景
    DroppableGraphicsView *canvasView;  // 自定义的、支持拖放的画布视图

    // 用于管理图形项堆叠顺序（Z值）的计数器，确保新添加或点击的项总在最上层
    qreal zCounter = 0;
};

#endif // STITCHERDIALOG_H
