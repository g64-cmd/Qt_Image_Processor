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
// File: newstitcherdialog.cpp
//
// Description:
// NewStitcherDialog 类的实现文件。该文件包含了新版图像拼接对话框的
// 具体业务逻辑，包括UI交互、线程管理和结果处理。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "newstitcherdialog.h"
#include "ui_newstitcherdialog.h"
#include "imageconverter.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QListWidgetItem>
#include <vector>
#include <QPushButton>
#include <QTimer>
#include <QDebug>

/**
 * @brief NewStitcherDialog 构造函数。
 *
 * 负责初始化UI，并特别处理了按钮盒（QDialogButtonBox）的行为，
 * 将其默认的"OK"按钮重新配置为"开始拼接"按钮。
 * @param parent 父窗口部件。
 */
NewStitcherDialog::NewStitcherDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewStitcherDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("基于特征点的图像拼接"));

    // --- 自定义按钮行为 ---
    // 默认情况下，点击OK按钮会触发accept()信号并关闭对话框。
    // 我们需要先断开这个默认连接，以便自定义其行为。
    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    // 获取OK按钮的指针，并修改其文本和点击事件的连接。
    QPushButton* stitchButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (stitchButton) {
        stitchButton->setText(tr("开始拼接"));
        connect(stitchButton, &QPushButton::clicked, this, &NewStitcherDialog::on_stitchButtonClicked);
    }

    // --- 初始化UI状态 ---
    ui->progressBar->setVisible(false);
    updateButtonStates(); // 根据初始状态（列表为空）设置按钮可用性
    connect(ui->imageListWidget, &QListWidget::itemSelectionChanged, this, &NewStitcherDialog::updateButtonStates);

    qDebug() << "NewStitcherDialog constructed.";
}

/**
 * @brief NewStitcherDialog 析构函数。
 */
NewStitcherDialog::~NewStitcherDialog()
{
    qDebug() << "NewStitcherDialog destructed.";
    delete ui;
}

/**
 * @brief 获取拼接后的最终图像。
 * @return 返回处理结果的 QPixmap。
 */
QPixmap NewStitcherDialog::getResultImage() const
{
    return resultPixmap;
}

/**
 * @brief 槽函数：响应“添加图片”按钮的点击。
 */
void NewStitcherDialog::on_addButton_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        tr("选择要拼接的图片 (按顺序)"),
        "",
        tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));

    if (!files.isEmpty()) {
        ui->imageListWidget->addItems(files);
        imagePaths.append(files);
        updateButtonStates();
    }
}

/**
 * @brief 槽函数：响应“移除图片”按钮的点击。
 */
void NewStitcherDialog::on_removeButton_clicked()
{
    QList<QListWidgetItem*> selectedItems = ui->imageListWidget->selectedItems();
    if (selectedItems.isEmpty()) return;

    for (QListWidgetItem* item : selectedItems) {
        imagePaths.removeOne(item->text());
        // takeItem会从列表中移除项并返回其指针，然后我们删除它以释放内存。
        delete ui->imageListWidget->takeItem(ui->imageListWidget->row(item));
    }
    updateButtonStates();
}

/**
 * @brief 槽函数：响应“上移”按钮的点击。
 */
void NewStitcherDialog::on_moveUpButton_clicked()
{
    int currentRow = ui->imageListWidget->currentRow();
    if (currentRow > 0) {
        // 同步操作UI列表和内部数据列表
        QListWidgetItem *item = ui->imageListWidget->takeItem(currentRow);
        ui->imageListWidget->insertItem(currentRow - 1, item);
        imagePaths.move(currentRow, currentRow - 1);
        ui->imageListWidget->setCurrentRow(currentRow - 1); // 保持选中状态
    }
}

/**
 * @brief 槽函数：响应“下移”按钮的点击。
 */
void NewStitcherDialog::on_moveDownButton_clicked()
{
    int currentRow = ui->imageListWidget->currentRow();
    if (currentRow < ui->imageListWidget->count() - 1) {
        // 同步操作UI列表和内部数据列表
        QListWidgetItem *item = ui->imageListWidget->takeItem(currentRow);
        ui->imageListWidget->insertItem(currentRow + 1, item);
        imagePaths.move(currentRow, currentRow + 1);
        ui->imageListWidget->setCurrentRow(currentRow + 1); // 保持选中状态
    }
}

/**
 * @brief 槽函数：响应“开始拼接”按钮的点击。
 */
void NewStitcherDialog::on_stitchButtonClicked()
{
    if (imagePaths.count() < 2) {
        QMessageBox::warning(this, tr("数量不足"), tr("请至少选择两张图片进行拼接。"));
        return;
    }

    // --- 1. 准备图像数据 ---
    std::vector<cv::Mat> images;
    for (const QString &path : imagePaths) {
        // 使用 toLocal8Bit().constData() 来正确处理可能包含非ASCII字符的文件路径
        cv::Mat img = cv::imread(path.toLocal8Bit().constData());
        if (img.empty()) {
            QMessageBox::critical(this, tr("加载失败"), QString("无法加载图片: %1").arg(path));
            return;
        }
        images.push_back(img);
    }

    // --- 2. 禁用UI，准备后台处理 ---
    ui->progressBar->setVisible(true);
    ui->buttonBox->setEnabled(false);
    ui->addButton->setEnabled(false);
    ui->removeButton->setEnabled(false);
    ui->moveUpButton->setEnabled(false);
    ui->moveDownButton->setEnabled(false);
    ui->imageListWidget->setEnabled(false);

    // --- 3. 创建并启动工作线程 ---
    StitcherThread* worker = new StitcherThread(images, this);

    // --- 4. 设置信号槽连接 ---
    // a. 连接线程的 resultReady 信号，用于在处理完成后接收结果
    connect(worker, &StitcherThread::resultReady, this, [this](const cv::Mat& resultMat) {
        qDebug() << "Thread finished processing, resultReady signal received.";
        ui->progressBar->setVisible(false);
        if (resultMat.empty()) {
            QMessageBox::critical(this, tr("拼接失败"), tr("无法拼接所选图片，请确保图片之间有足够的重叠区域且顺序正确。"));
        } else {
            resultPixmap = QPixmap::fromImage(ImageConverter::matToQImage(resultMat));
            ui->previewLabel->setPixmap(resultPixmap.scaled(ui->previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    });

    // b. 连接线程的 finished 信号，用于在线程结束后恢复UI状态
    connect(worker, &QThread::finished, this, [this](){
        qDebug() << "Thread finished signal received. UI is now safe to modify.";
        ui->buttonBox->setEnabled(true);
        ui->imageListWidget->setEnabled(true);
        updateButtonStates();
        // 如果拼接成功，则将"开始拼接"按钮恢复为"确定"按钮，并连接accept()槽
        if (!resultPixmap.isNull()) {
            QPushButton* okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
            if (okButton) {
                okButton->setText(tr("确定"));
                disconnect(okButton, &QPushButton::clicked, this, &NewStitcherDialog::on_stitchButtonClicked);
                connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
            }
        }
    });

    // c. 连接线程的 finished 信号到自身的 deleteLater 槽，以安全地释放线程对象
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);

    qDebug() << "Starting stitcher thread...";
    worker->start();
}

/**
 * @brief 根据列表中的选择状态和数量更新按钮的可用性。
 */
void NewStitcherDialog::updateButtonStates()
{
    bool hasSelection = !ui->imageListWidget->selectedItems().isEmpty();
    ui->removeButton->setEnabled(hasSelection);
    ui->moveUpButton->setEnabled(hasSelection);
    ui->moveDownButton->setEnabled(hasSelection);
    ui->addButton->setEnabled(true);

    // "开始拼接"按钮只有在图片数量大于等于2时才可用
    QPushButton* stitchButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (stitchButton) {
        stitchButton->setEnabled(ui->imageListWidget->count() >= 2);
    }
}
