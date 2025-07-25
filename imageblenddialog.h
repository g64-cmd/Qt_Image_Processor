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

#ifndef IMAGEBLENDDIALOG_H
#define IMAGEBLENDDIALOG_H

// =============================================================================
// File: imageblenddialog.h
//
// Description:
// 该文件定义了 ImageBlendDialog 类，这是一个用于图像融合的对话框。
// 用户可以加载第二张图片，并通过滑块调整两张图片的混合比例。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QDialog>
#include <QPixmap>

// --- 前置声明 ---
namespace Ui {
class ImageBlendDialog;
}

/**
 * @class ImageBlendDialog
 * @brief 提供图像融合功能的设置对话框。
 *
 * 该对话框允许用户加载一张待融合的图像，并使用滑块实时调整
 * 两张图像的混合权重，预览并获取最终的融合结果。
 */
class ImageBlendDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     * @param initialPixmap 待融合的第一张图像 (图像A)。
     * @param parent 父窗口部件，默认为nullptr。
     */
    explicit ImageBlendDialog(const QPixmap &initialPixmap, QWidget *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~ImageBlendDialog();

    /**
     * @brief 获取经过融合处理后的最终图像。
     * @return 返回处理结果的 QPixmap。
     */
    QPixmap getBlendedImage() const;

private slots:
    /**
     * @brief 槽函数：响应“打开图像B”按钮的点击事件。
     */
    void on_buttonOpenImageB_clicked();

    /**
     * @brief 槽函数：响应混合比例滑块值的变化。
     * @param value 新的混合比例值。
     */
    void on_sliderBlend_valueChanged(int value);

private:
    /**
     * @brief 根据当前滑块值更新融合后的图像预览。
     */
    void updateBlendedImage();

    // --- 成员变量 ---
    Ui::ImageBlendDialog *ui;   // Qt Designer生成的UI类实例
    QPixmap pixmapA;            // 存储第一张待融合的图像
    QPixmap pixmapB;            // 存储第二张待融合的图像
    QPixmap blendedPixmap;      // 存储当前融合后的结果图像
};

#endif // IMAGEBLENDDIALOG_H
