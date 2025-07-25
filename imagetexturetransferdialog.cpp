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
// File: imagetexturetransferdialog.cpp
//
// Description:
// ImageTextureTransferDialog 类的实现文件。该文件包含了纹理迁移
// 对话框的具体业务逻辑。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "imagetexturetransferdialog.h"
#include "ui_imagetexturetransferdialog.h"
#include "imageprocessor.h" // 假设 ImageProcessor 提供了静态的 textureTransfer 方法
#include <QFileDialog>
#include <QPushButton>
#include <QApplication>

/**
 * @brief ImageTextureTransferDialog 构造函数。
 *
 * 负责初始化UI，显示内容图像，并设置按钮和进度条的初始状态。
 * @param contentPixmap 作为基础的内容图像。
 * @param parent 父窗口部件。
 */
ImageTextureTransferDialog::ImageTextureTransferDialog(const QPixmap &contentPixmap, QWidget *parent) :
    QDialog(parent),
    // [修正] 此处使用的类名必须与 .h 文件和 uic 生成的类名完全匹配
    ui(new Ui::imagetexturetransferdialog),
    contentPixmap(contentPixmap)
{
    // --- 1. UI 初始化 ---
    ui->setupUi(this);
    setWindowTitle(tr("纹理迁移"));

    // --- 2. 初始状态设置 ---
    // 在 "Content" 标签中显示内容图像的缩略图
    ui->labelContent->setPixmap(contentPixmap.scaled(ui->labelContent->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // --- 3. 按钮连接 ---
    QPushButton *applyButton = ui->buttonBox->button(QDialogButtonBox::Apply);
    if (applyButton) {
        connect(applyButton, &QPushButton::clicked, this, &ImageTextureTransferDialog::accept);
    }

    // --- 4. 进度条设置 ---
    // 初始时隐藏进度条
    ui->progressBar->setVisible(false);
}

/**
 * @brief ImageTextureTransferDialog 析构函数。
 */
ImageTextureTransferDialog::~ImageTextureTransferDialog()
{
    delete ui;
}

/**
 * @brief 获取经过纹理迁移处理后的最终图像。
 * @return 返回处理结果的 QPixmap。
 */
QPixmap ImageTextureTransferDialog::getResultImage() const
{
    return resultPixmap;
}

/**
 * @brief 槽函数：响应“打开纹理图片”按钮的点击事件。
 *
 * 弹出文件对话框让用户选择纹理图像，加载成功后立即开始处理。
 */
void ImageTextureTransferDialog::on_buttonOpenTexture_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择纹理图"), "", "Image Files (*.png *.jpg *.bmp)");
    if (!fileName.isEmpty()) {
        texturePixmap.load(fileName);
        // 在 "Texture" 标签中显示纹理图像的缩略图
        ui->labelTexture->setPixmap(texturePixmap.scaled(ui->labelTexture->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        // 加载新纹理后，立即应用迁移算法
        applyTextureTransfer();
    }
}

/**
 * @brief 应用纹理迁移算法并更新UI。
 *
 * 这是一个耗时操作。为了防止UI冻结，在调用处理函数前后更新了UI状态，
 * 并使用 QApplication::processEvents() 来强制处理UI事件。
 */
void ImageTextureTransferDialog::applyTextureTransfer()
{
    if (contentPixmap.isNull() || texturePixmap.isNull()) return;

    // --- 准备处理：更新UI以提供反馈 ---
    ui->labelResult->setText(tr("正在处理中，请稍后！"));
    ui->progressBar->setVisible(true);
    // 设置为不确定模式（滚动条）
    ui->progressBar->setRange(0, 0);
    // 强制Qt处理挂起的事件，确保UI（标签文本和进度条）能够立即更新
    QApplication::processEvents();

    // --- 执行耗时操作 ---
    QImage result = ImageProcessor::textureTransfer(contentPixmap.toImage(), texturePixmap.toImage());

    // --- 处理完成：更新UI ---
    ui->progressBar->setVisible(false);
    if (!result.isNull()) {
        resultPixmap = QPixmap::fromImage(result);
        ui->labelResult->setPixmap(resultPixmap.scaled(ui->labelResult->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        ui->labelResult->setText(tr("处理失败"));
    }
}
