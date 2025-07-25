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

#ifndef STAGINGAREAMANAGER_H
#define STAGINGAREAMANAGER_H

// =============================================================================
// File: stagingareamanager.h
//
// Description:
// 该文件定义了 StagingAreaManager 类，负责管理应用程序的暂存区。
// 暂存区是一个临时的图像存储空间，用户处理过的所有图像版本都会
// 在这里显示，方便用户随时切换和比较。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QObject>
#include <QPixmap>
#include <QList>
#include <QString> // 包含 QString 的定义

// --- 前置声明 ---
class DraggableItemModel;

/**
 * @class StagingAreaManager
 * @brief 管理暂存区图像的业务逻辑。
 *
 * 此类维护一个图像列表，处理图像的添加、更新、删除和排序。
 * 它与 DraggableItemModel 紧密协作，将内部数据同步到UI视图中。
 */
class StagingAreaManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @struct StagedImage
     * @brief 用于存储暂存区中单个图像所有信息的结构体。
     */
    struct StagedImage {
        QString id;     // 图像的唯一标识符
        QString name;   // 显示在UI上的名称
        QPixmap pixmap; // 图像数据
    };

    /**
     * @brief 构造函数。
     * @param model 指向与此管理器关联的数据模型，用于更新UI。
     * @param parent 父对象。
     */
    explicit StagingAreaManager(DraggableItemModel *model, QObject *parent = nullptr);

    /**
     * @brief 向暂存区添加一张新图像。
     * @param pixmap 要添加的图像。
     * @param baseName 图像的基础名称。
     * @return 返回新图像的唯一ID。
     */
    QString addNewImage(const QPixmap &pixmap, const QString &baseName);

    /**
     * @brief 更新暂存区中指定ID的图像。
     * @param id 要更新的图像的ID。
     * @param newPixmap 新的图像数据。
     */
    void updateImage(const QString &id, const QPixmap &newPixmap);

    /**
     * @brief 将指定ID的图像提升到列表顶部（最近使用的位置）。
     * @param id 要提升的图像的ID。
     */
    void promoteImage(const QString &id);

    /**
     * @brief 获取指定ID的图像数据。
     * @param id 图像ID。
     * @return 返回对应的 QPixmap。
     */
    QPixmap getPixmap(const QString &id) const;

    /**
     * @brief 获取指定ID的完整暂存图像信息（包括名称）。
     * @param id 图像ID。
     * @return 返回 StagedImage 结构体。
     */
    StagedImage getStagedImage(const QString &id) const;

    /**
     * @brief 获取暂存区中的图像总数。
     * @return 图像数量。
     */
    int getImageCount() const;

    /**
     * @brief 从暂存区中移除指定ID的图像。
     * @param id 要移除的图像的ID。
     */
    void removeImage(const QString &id);

private:
    /**
     * @brief 更新数据模型，将内部数据列表同步到UI视图。
     */
    void updateModel();

    // --- 成员变量 ---
    DraggableItemModel *model;      // 指向UI视图的数据模型
    QList<StagedImage> stagedImages; // 存储所有暂存图像的内部列表

    // 暂存区最大容量
    static const int MaxStagedImages = 15;
    // 用于生成唯一ID的计数器
    int imageCounter = 0;
};

#endif // STAGINGAREAMANAGER_H
