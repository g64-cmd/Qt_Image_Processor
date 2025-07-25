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
// File: stitcherdialog.cpp
//
// Description:
// StitcherDialog 类的实现文件。该文件包含了手动图像拼接对话框的
// 具体业务逻辑，包括UI的动态设置、场景管理、拖放处理和键盘事件响应。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

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

/**
 * @brief StitcherDialog 构造函数。
 *
 * 负责初始化UI，动态创建并设置画布（QGraphicsView 和 QGraphicsScene），
 * 并连接所有必要的信号和槽。
 * @param manager 指向 StagingAreaManager 的指针。
 * @param model 指向暂存区的数据模型。
 * @param parent 父窗口部件。
 */
StitcherDialog::StitcherDialog(StagingAreaManager *manager, DraggableItemModel *model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StitcherDialog),
    stagingManager(manager),
    sourceModel(model),
    zCounter(0.0)
{
    ui->setupUi(this);
    setWindowTitle(tr("图像拼接画布"));

    // --- 动态创建和设置画布 ---
    // 1. 创建自定义的 DroppableGraphicsView 实例
    canvasView = new DroppableGraphicsView(this);
    // 2. 创建一个布局来管理占位符控件
    QVBoxLayout *canvasLayout = new QVBoxLayout(ui->canvasPlaceholder);
    canvasLayout->setContentsMargins(0, 0, 0, 0); // 移除边距，让画布填满
    // 3. 将自定义视图添加到布局中
    canvasLayout->addWidget(canvasView);

    // --- 设置图形场景 ---
    scene = new QGraphicsScene(this);
    canvasView->setScene(scene);
    // 设置一个较大的场景矩形，为用户提供充足的拼接空间
    scene->setSceneRect(-1000, -1000, 2000, 2000);

    // --- 设置源图像列表视图 ---
    ui->sourceImagesView->setModel(sourceModel);
    ui->sourceImagesView->setViewMode(QListView::IconMode);
    ui->sourceImagesView->setIconSize(QSize(100, 100));
    ui->sourceImagesView->setResizeMode(QListView::Adjust);
    ui->sourceImagesView->setWordWrap(true);
    ui->sourceImagesView->setDragEnabled(true); // 启用拖动

    // --- 连接信号和槽 ---
    connect(canvasView, &DroppableGraphicsView::stagedImageDropped, this, &StitcherDialog::onStagedImageDropped);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

/**
 * @brief StitcherDialog 析构函数。
 */
StitcherDialog::~StitcherDialog()
{
    delete ui;
}

/**
 * @brief 获取最终手动拼接的图像。
 *
 * 将场景中所有可见项渲染到一张新的 QPixmap 上。
 * @return 返回最终的拼接结果图像。
 */
QPixmap StitcherDialog::getFinalImage() const
{
    // 1. 计算包含场景中所有项的最小边界矩形
    QRectF bounds = scene->itemsBoundingRect();
    if (bounds.isEmpty()) {
        return QPixmap(); // 如果场景为空，返回空图像
    }

    // 2. 创建一个与边界矩形大小相同、支持透明度的图像
    QImage image(bounds.size().toSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent); // 用透明色填充背景

    // 3. 使用QPainter将场景内容渲染到图像上
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing); // 启用抗锯齿以获得更高质量
    // scene->render 的最后一个参数指定了只渲染场景中的哪个源区域
    scene->render(&painter, QRectF(), bounds);
    painter.end();

    return QPixmap::fromImage(image);
}

/**
 * @brief 槽函数：当一个暂存区图像被拖放到画布上时调用。
 * @param imageId 被拖放的图像的唯一ID。
 */
void StitcherDialog::onStagedImageDropped(const QString &imageId)
{
    if (imageId.isEmpty()) return;
    QPixmap pixmap = stagingManager->getPixmap(imageId);
    if (pixmap.isNull()) return;

    // 创建一个新的可交互图像项
    InteractivePixmapItem *item = new InteractivePixmapItem(pixmap);
    scene->addItem(item);

    // 连接信号，以便在该项被点击时可以将其置顶
    connect(item, &InteractivePixmapItem::itemClicked, this, &StitcherDialog::bringItemToFront);

    // 将新添加的项置顶
    bringItemToFront(item);

    // 将项放置在视图的中心位置
    item->setPos(canvasView->mapToScene(canvasView->viewport()->rect().center()));
}

/**
 * @brief 槽函数：将被点击的图像项带到最顶层显示。
 *
 * 通过增加项的 Z 值来实现。Z值越大的项显示在越上层。
 * @param item 被点击的 InteractivePixmapItem 指针。
 */
void StitcherDialog::bringItemToFront(InteractivePixmapItem *item)
{
    zCounter += 1.0; // 递增全局Z值计数器
    item->setZValue(zCounter); // 将新的、更大的Z值赋给项
}

/**
 * @brief 键盘按下事件处理器。
 *
 * 实现了按住 Shift + A/D 键来旋转选中的图像项。
 * @param event 键盘事件。
 */
void StitcherDialog::keyPressEvent(QKeyEvent *event)
{
    // 检查是否按下了Shift修饰键
    if (event->modifiers() == Qt::ShiftModifier) {
        QList<QGraphicsItem*> selected = scene->selectedItems();
        if (selected.isEmpty()) {
            // 如果没有项被选中，则调用基类实现处理默认事件（如按Esc关闭）
            QDialog::keyPressEvent(event);
            return;
        }

        qreal rotationAngle = 5.0; // 每次旋转5度
        if (event->key() == Qt::Key_A) { // Shift + A: 逆时针旋转
            for (QGraphicsItem *item : selected) {
                item->setRotation(item->rotation() - rotationAngle);
            }
            event->accept(); // 事件已处理
            return;
        }
        if (event->key() == Qt::Key_D) { // Shift + D: 顺时针旋转
            for (QGraphicsItem *item : selected) {
                item->setRotation(item->rotation() + rotationAngle);
            }
            event->accept(); // 事件已处理
            return;
        }
    }

    // 如果不是我们定义的快捷键，则调用基类实现
    QDialog::keyPressEvent(event);
}
