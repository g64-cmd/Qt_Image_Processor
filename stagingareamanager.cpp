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
// File: stagingareamanager.cpp
//
// Description:
// StagingAreaManager 类的实现文件。该文件包含了管理暂存区图像的
// 所有业务逻辑，包括增、删、改、查和排序。
//
// Author: g64-cmd
// Date: 2025-07-25
// =============================================================================

#include "stagingareamanager.h"
#include "draggableitemmodel.h"
#include <QStandardItem>
#include <QIcon>
#include <QUuid>
#include <utility> // For std::as_const

/**
 * @brief StagingAreaManager 构造函数。
 * @param model 指向UI视图的数据模型。
 * @param parent 父对象。
 */
StagingAreaManager::StagingAreaManager(DraggableItemModel *model, QObject *parent)
    : QObject(parent), model(model)
{
    // 确保传入的模型指针是有效的，这是该类正常工作的前提。
    Q_ASSERT(model != nullptr);
}

/**
 * @brief 向暂存区添加一张新图像。
 *
 * 新图像会被添加到列表的最前端。如果列表超出最大容量，
 * 最旧的图像（列表末尾的）将被移除。
 * @param pixmap 要添加的图像。
 * @param baseName 图像的基础名称。
 * @return 返回新图像的唯一ID。
 */
QString StagingAreaManager::addNewImage(const QPixmap &pixmap, const QString &baseName)
{
    if (pixmap.isNull()) return QString();

    StagedImage newImage;
    // 使用QUuid生成一个全局唯一的ID
    newImage.id = QUuid::createUuid().toString();
    newImage.pixmap = pixmap;
    // 创建一个唯一的名称，例如 "myImage_1", "myImage_2"
    newImage.name = QString("%1_%2").arg(baseName).arg(++imageCounter);

    // 将新项插入到列表的开头
    stagedImages.prepend(newImage);

    // 维持暂存区的最大容量
    while (stagedImages.size() > MaxStagedImages) {
        stagedImages.removeLast();
    }

    // 更新UI
    updateModel();
    return newImage.id;
}

/**
 * @brief 更新暂存区中指定ID的图像，并将其移到最前面。
 * @param id 要更新的图像的ID。
 * @param newPixmap 新的图像数据。
 */
void StagingAreaManager::updateImage(const QString &id, const QPixmap &newPixmap)
{
    int foundIndex = -1;
    for (int i = 0; i < stagedImages.size(); ++i) {
        if (stagedImages[i].id == id) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex == -1) return; // 未找到则不执行任何操作

    // 将找到的项从列表中取出，更新其内容，然后重新插入到列表头部
    StagedImage item = stagedImages.takeAt(foundIndex);
    item.pixmap = newPixmap;
    stagedImages.prepend(item);

    updateModel();
}

/**
 * @brief 将指定ID的图像提升到列表顶部（最近使用的位置）。
 *
 * 当用户点击或选择一个已存在的暂存项时调用此函数。
 * @param id 要提升的图像的ID。
 */
void StagingAreaManager::promoteImage(const QString &id)
{
    int foundIndex = -1;
    for (int i = 0; i < stagedImages.size(); ++i) {
        if (stagedImages[i].id == id) {
            foundIndex = i;
            break;
        }
    }

    // 如果项未找到，或者已经位于列表顶部，则无需操作
    if (foundIndex <= 0) return;

    // 将找到的项从当前位置取出，并重新插入到列表头部
    StagedImage item = stagedImages.takeAt(foundIndex);
    stagedImages.prepend(item);

    updateModel();
}

/**
 * @brief 获取指定ID的图像数据 (QPixmap)。
 * @param id 图像ID。
 * @return 返回对应的 QPixmap。如果未找到，则返回一个空的QPixmap。
 */
QPixmap StagingAreaManager::getPixmap(const QString &id) const
{
    for (const auto &image : std::as_const(stagedImages)) {
        if (image.id == id) {
            return image.pixmap;
        }
    }
    return QPixmap();
}

/**
 * @brief 获取指定ID的完整暂存图像信息（包括名称）。
 * @param id 图像ID。
 * @return 返回 StagedImage 结构体。如果未找到，则返回一个默认构造的空结构体。
 */
StagingAreaManager::StagedImage StagingAreaManager::getStagedImage(const QString &id) const
{
    for (const auto &image : std::as_const(stagedImages)) {
        if (image.id == id) {
            return image;
        }
    }
    return StagedImage();
}

/**
 * @brief 获取暂存区中的图像总数。
 * @return 图像数量。
 */
int StagingAreaManager::getImageCount() const
{
    return stagedImages.size();
}

/**
 * @brief 从暂存区中移除指定ID的图像。
 * @param id 要移除的图像的ID。
 */
void StagingAreaManager::removeImage(const QString &id)
{
    // 使用C++11的lambda表达式和removeIf高效地移除匹配项
    stagedImages.removeIf([&](const StagedImage &img){
        return img.id == id;
    });
    updateModel();
}

/**
 * @brief 更新数据模型，将内部数据列表同步到UI视图。
 *
 * 这是连接业务逻辑和用户界面的核心函数。
 */
void StagingAreaManager::updateModel()
{
    if (!model) return;

    // 清空模型中的所有旧项
    model->clear();
    // 遍历内部的stagedImages列表
    for (const auto &image : std::as_const(stagedImages)) {
        // 为每个图像创建一个新的QStandardItem
        QStandardItem *item = new QStandardItem();
        // 设置项的图标（缩略图）
        item->setIcon(QIcon(image.pixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        // 设置项的显示文本
        item->setText(image.name);
        // 将图像的唯一ID存储在UserRole中，这是一个不可见的数据角色，用于拖放和识别
        item->setData(image.id, Qt::UserRole);
        // 将新项添加到模型中
        model->appendRow(item);
    }
}
