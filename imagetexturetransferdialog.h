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

#ifndef IMAGETEXTURETRANSFERDIALOG_H
#define IMAGETEXTURETRANSFERDIALOG_H

// =============================================================================
// File: imagetexturetransferdialog.h
//
// Description:
// 该文件定义了 ImageTextureTransferDialog 类，这是一个用于
// 图像纹理迁移的对话框。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QDialog>
#include <QPixmap>

// --- 前置声明 ---
namespace Ui {
// [修正] uic生成的类名与.ui文件中的objectName一致，此处为小写'i'
class imagetexturetransferdialog;
}

/**
 * @class ImageTextureTransferDialog
 * @brief 提供图像纹理迁移功能的设置对话框。
 *
 * 该对话框允许用户加载一张纹理图像，并将其纹理应用到
 * 另一张内容图像上，预览并获取最终结果。
 */
class ImageTextureTransferDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     * @param contentPixmap 作为基础的内容图像。
     * @param parent 父窗口部件，默认为nullptr。
     */
    explicit ImageTextureTransferDialog(const QPixmap &contentPixmap, QWidget *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~ImageTextureTransferDialog();

    /**
     * @brief 获取经过纹理迁移处理后的最终图像。
     * @return 返回处理结果的 QPixmap。
     */
    QPixmap getResultImage() const;

private slots:
    /**
     * @brief 槽函数：响应“打开纹理图片”按钮的点击事件。
     */
    void on_buttonOpenTexture_clicked();

private:
    /**
     * @brief 应用纹理迁移算法并更新预览。
     */
    void applyTextureTransfer();

    // --- 成员变量 ---
    // [修正] ui指针的类型必须与Ui命名空间中声明的类完全匹配
    Ui::imagetexturetransferdialog *ui;
    QPixmap contentPixmap;
    QPixmap texturePixmap;
    QPixmap resultPixmap;
};

#endif // IMAGETEXTURETRANSFERDIALOG_H
