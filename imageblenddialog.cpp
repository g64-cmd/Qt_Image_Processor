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
// File: imageblenddialog.cpp
//
// Description:
// ImageBlendDialog 类的实现文件。该文件包含了图像融合对话框的
// 具体业务逻辑。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "imageblenddialog.h"
#include "ui_imageblenddialog.h"
#include "imageprocessor.h" // 假设 ImageProcessor 提供了静态的 blend 方法
#include <QFileDialog>
#include <QPushButton>

/**
 * @brief ImageBlendDialog 构造函数。
 *
 * 负责初始化UI，显示第一张待融合的图像（图像A），并设置滑块的初始状态。
 * @param initialPixmap 待融合的第一张图像。
 * @param parent 父窗口部件。
 */
ImageBlendDialog::ImageBlendDialog(const QPixmap &initialPixmap, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageBlendDialog),
    pixmapA(initialPixmap)
{
    // --- 1. UI 初始化 ---
    ui->setupUi(this);
    setWindowTitle(tr("图像融合"));

    // --- 2. 初始状态设置 ---
    // 在 "Image A" 标签中显示第一张图像的缩略图
    ui->labelImageA->setPixmap(pixmapA.scaled(ui->labelImageA->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    // 设置混合比例滑块的范围 (0% to 100%) 和初始值 (50%)
    ui->sliderBlend->setRange(0, 100);
    ui->sliderBlend->setValue(50);

    // --- 3. 按钮连接 ---
    // 将对话框按钮盒中的 "Apply" 按钮（如果存在）连接到 accept() 槽
    QPushButton *applyButton = ui->buttonBox->button(QDialogButtonBox::Apply);
    if (applyButton) {
        connect(applyButton, &QPushButton::clicked, this, &ImageBlendDialog::accept);
    }

    // --- 4. 初始预览 ---
    // 调用一次更新函数，以在对话框打开时就显示初始的融合结果
    updateBlendedImage();
}

/**
 * @brief ImageBlendDialog 析构函数。
 */
ImageBlendDialog::~ImageBlendDialog()
{
    delete ui;
}

/**
 * @brief 获取经过融合处理后的最终图像。
 * @return 返回处理结果的 QPixmap。
 */
QPixmap ImageBlendDialog::getBlendedImage() const
{
    return blendedPixmap;
}

/**
 * @brief 槽函数：响应“打开图像B”按钮的点击事件。
 *
 * 弹出文件对话框让用户选择第二张待融合的图像。
 */
void ImageBlendDialog::on_buttonOpenImageB_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择图片 B"), "", "Image Files (*.png *.jpg *.bmp)");
    if (!fileName.isEmpty()) {
        pixmapB.load(fileName);
        // 在 "Image B" 标签中显示第二张图像的缩略图
        ui->labelImageB->setPixmap(pixmapB.scaled(ui->labelImageB->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        // 加载新图后，立即更新融合结果预览
        updateBlendedImage();
    }
}

/**
 * @brief 槽函数：响应混合比例滑块值的变化。
 *
 * 每次滑块值改变时，都重新计算并更新融合图像。
 * @param value 新的混合比例值（在此函数中未使用，但槽函数签名需要）。
 */
void ImageBlendDialog::on_sliderBlend_valueChanged(int value)
{
    Q_UNUSED(value);
    updateBlendedImage();
}

/**
 * @brief 根据当前滑块值更新融合后的图像预览。
 *
 * 这是实现实时预览的核心函数。
 */
void ImageBlendDialog::updateBlendedImage()
{
    if (pixmapA.isNull()) return;

    // 如果图像B尚未加载，则结果就是图像A
    if (pixmapB.isNull()) {
        blendedPixmap = pixmapA;
    } else {
        // 从滑块获取混合权重 alpha (0.0 to 1.0)
        double alpha = ui->sliderBlend->value() / 100.0;
        // 调用处理器执行图像融合算法
        // 公式通常为：result = alpha * imageA + (1 - alpha) * imageB
        QImage resultImage = ImageProcessor::blend(pixmapA.toImage(), pixmapB.toImage(), alpha);
        if (!resultImage.isNull()) {
            blendedPixmap = QPixmap::fromImage(resultImage);
        }
    }

    // 更新UI上的结果预览
    if (!blendedPixmap.isNull()) {
        ui->labelResult->setPixmap(blendedPixmap.scaled(ui->labelResult->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}
