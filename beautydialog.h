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

#ifndef BEAUTYDIALOG_H
#define BEAUTYDIALOG_H

// =============================================================================
// File: beautydialog.h
//
// Description:
// 该文件定义了 BeautyDialog 类，一个用于实时调整美颜效果（如磨皮、瘦脸）
// 的对话框。用户可以通过滑块调整参数，并实时预览效果。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QDialog>
#include <QPixmap>

// --- 前置声明 ---
namespace Ui {
class BeautyDialog;
}
class BeautyProcessor; // 美颜算法的核心处理类

/**
 * @class BeautyDialog
 * @brief 提供美颜功能的设置对话框。
 *
 * 该对话框允许用户通过UI控件（滑块）来调整美颜滤镜的参数，
 * 如磨皮程度和瘦脸强度，并实时显示处理后的图像。
 */
class BeautyDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     * @param initialPixmap 需要进行美颜处理的原始图像。
     * @param parent 父窗口部件，默认为nullptr。
     */
    explicit BeautyDialog(const QPixmap &initialPixmap, QWidget *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~BeautyDialog();

    /**
     * @brief 获取经过美颜处理后的最终图像。
     * @return 返回处理结果的 QPixmap。
     */
    QPixmap getResultImage() const;

private slots:
    /**
     * @brief 槽函数：响应磨皮滑块值的变化。
     * @param value 新的磨皮值。
     */
    void on_sliderSmooth_valueChanged(int value);

    /**
     * @brief 槽函数：响应瘦脸滑块值的变化。
     * @param value 新的瘦脸值。
     */
    void on_sliderThin_valueChanged(int value);

private:
    /**
     * @brief 应用当前滑块设置的美颜滤镜。
     *
     * 该函数会获取所有滑块的当前值，并调用 BeautyProcessor 来处理图像，
     * 然后更新UI以显示结果。
     */
    void applyBeautyFilter();

    // --- 成员变量 ---
    Ui::BeautyDialog *ui;          // Qt Designer生成的UI类实例
    BeautyProcessor *processor;    // 负责执行美颜算法的处理对象
    QPixmap originalPixmap;        // 存储传入的原始图像，用于每次重新计算效果
    QPixmap resultPixmap;          // 存储当前处理后的结果图像
};

#endif // BEAUTYDIALOG_H
