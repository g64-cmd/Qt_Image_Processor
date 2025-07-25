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
// File: beautydialog.cpp
//
// Description:
// BeautyDialog 类的实现文件。该文件包含了美颜设置对话框的所有逻辑，
// 包括UI初始化、响应用户输入（如拖动滑块）以及调用 BeautyProcessor
// 来执行实际的图像处理。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "beautydialog.h"
#include "ui_beautydialog.h"
#include "beautyprocessor.h"

#include <QPushButton>

/**
 * @brief BeautyDialog 构造函数。
 *
 * 负责初始化UI，创建美颜处理器，设置滑块范围和默认值，
 * 并显示初始的“处理前”预览图像。
 * @param initialPixmap 需要进行美颜处理的原始图像。
 * @param parent 父窗口部件。
 */
BeautyDialog::BeautyDialog(const QPixmap &initialPixmap, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BeautyDialog),
    originalPixmap(initialPixmap)
{
    // --- 1. UI 初始化 ---
    ui->setupUi(this);
    setWindowTitle(tr("美颜工作室"));

    // --- 2. 处理器和预览设置 ---
    processor = new BeautyProcessor();
    // 在 "Before" 标签中显示原始图像的缩略图
    ui->labelBefore->setPixmap(originalPixmap.scaled(ui->labelBefore->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // --- 3. 滑块设置 ---
    // 设置磨皮滑块的范围和初始值
    ui->sliderSmooth->setRange(0, 100);
    ui->sliderSmooth->setValue(50);
    // 设置瘦脸滑块的范围和初始值
    ui->sliderThin->setRange(0, 100);
    ui->sliderThin->setValue(0);

    // --- 4. 按钮连接 ---
    // 将对话框按钮盒中的 "Apply" 按钮（如果存在）连接到 accept() 槽
    // accept() 会关闭对话框并返回 QDialog::Accepted
    QPushButton *applyButton = ui->buttonBox->button(QDialogButtonBox::Apply);
    if (applyButton) {
        connect(applyButton, &QPushButton::clicked, this, &BeautyDialog::accept);
    }

    // --- 5. 初始处理 ---
    // 立即应用一次默认的滤镜效果，以生成初始的“处理后”预览
    applyBeautyFilter();
}

/**
 * @brief BeautyDialog 析构函数。
 *
 * 清理UI对象和美颜处理器实例。
 */
BeautyDialog::~BeautyDialog()
{
    delete ui;
    delete processor;
}

/**
 * @brief 获取经过美颜处理后的最终图像。
 *
 * 该函数在对话框被接受后由外部调用，以获取最终的处理结果。
 * @return 返回处理结果的 QPixmap。
 */
QPixmap BeautyDialog::getResultImage() const
{
    return resultPixmap;
}

/**
 * @brief 槽函数：响应磨皮滑块值的变化。
 *
 * 每次滑块值改变时，都重新应用美颜滤镜以更新预览。
 * @param value 新的磨皮值（在此函数中未使用，但槽函数签名需要）。
 */
void BeautyDialog::on_sliderSmooth_valueChanged(int value)
{
    // (void)value; 用于抑制编译器关于 "未使用参数" 的警告。
    (void)value;
    applyBeautyFilter();
}

/**
 * @brief 槽函数：响应瘦脸滑块值的变化。
 *
 * 每次滑块值改变时，都重新应用美颜滤镜以更新预览。
 * @param value 新的瘦脸值（在此函数中未使用，但槽函数签名需要）。
 */
void BeautyDialog::on_sliderThin_valueChanged(int value)
{
    (void)value;
    applyBeautyFilter();
}

/**
 * @brief 应用当前滑块设置的美颜滤镜。
 *
 * 这是实现实时预览的核心函数。它从UI读取参数，调用处理器，
 * 并将结果显示在 "After" 标签中。
 */
void BeautyDialog::applyBeautyFilter()
{
    // 1. 从UI获取当前的参数值
    int smoothLevel = ui->sliderSmooth->value();
    int thinLevel = ui->sliderThin->value();

    // 2. 调用处理器执行美颜算法
    // 注意：每次都从 originalPixmap 开始处理，以避免效果叠加
    QImage result = processor->process(originalPixmap.toImage(), smoothLevel, thinLevel);

    // 3. 更新结果图像和UI预览
    if (!result.isNull()) {
        resultPixmap = QPixmap::fromImage(result);
        // 在 "After" 标签中显示处理后的图像缩略图
        // Qt::SmoothTransformation 提供了更高质量的缩放效果
        ui->labelAfter->setPixmap(resultPixmap.scaled(ui->labelAfter->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}
