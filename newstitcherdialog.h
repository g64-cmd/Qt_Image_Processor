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

#ifndef NEWSTITCHERDIALOG_H
#define NEWSTITCHERDIALOG_H

// =============================================================================
// File: newstitcherdialog.h
//
// Description:
// 该文件定义了 NewStitcherDialog 类和一个辅助的 StitcherThread 类。
// NewStitcherDialog 提供了一个新的用户界面来管理和执行图像拼接任务，
// 而 StitcherThread 则负责在后台线程中运行耗时的拼接算法，以避免
// 阻塞用户界面。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QDialog>
#include <QStringList>
#include <QPixmap>
#include <QThread>
#include <opencv2/opencv.hpp>
#include "imagestitcherprocessor.h" // 包含拼接处理器

/**
 * @class StitcherThread
 * @brief 在后台执行图像拼接的线程类。
 *
 * 继承自 QThread，将耗时的 ImageStitcherProcessor::process 调用
 * 移出主GUI线程，防止界面冻结。
 */
class StitcherThread : public QThread
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     * @param imgs 待拼接的图像列表 (cv::Mat)。
     * @param parent 父对象。
     */
    StitcherThread(const std::vector<cv::Mat>& imgs, QObject* parent = nullptr)
        : QThread(parent), images(imgs) {}

protected:
    /**
     * @brief 线程的入口点。
     *
     * 当调用 QThread::start() 时，此函数会在新的线程中被执行。
     */
    void run() override {
        ImageStitcherProcessor processor;
        cv::Mat result = processor.process(images);
        // 当处理完成后，发射信号将结果传递回主线程
        emit resultReady(result);
    }

signals:
    /**
     * @brief 当拼接处理完成时发射此信号。
     * @param image 拼接后的结果图像 (cv::Mat)。
     */
    void resultReady(const cv::Mat& image);

private:
    // --- 成员变量 ---
    std::vector<cv::Mat> images; // 存储待处理的图像数据
};


// --- 前置声明 ---
namespace Ui {
class NewStitcherDialog;
}

/**
 * @class NewStitcherDialog
 * @brief 新版图像拼接对话框。
 *
 * 提供一个列表视图来管理待拼接的图像文件，用户可以添加、移除和
 * 调整图像顺序，然后启动拼接过程。
 */
class NewStitcherDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     * @param parent 父窗口部件。
     */
    explicit NewStitcherDialog(QWidget *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~NewStitcherDialog();

    /**
     * @brief 获取拼接后的最终图像。
     * @return 返回处理结果的 QPixmap。
     */
    QPixmap getResultImage() const;

private slots:
    // --- UI交互槽函数 ---
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_moveUpButton_clicked();
    void on_moveDownButton_clicked();
    void on_stitchButtonClicked();

private:
    /**
     * @brief 根据列表中的选择状态更新按钮（上移/下移/移除）的可用性。
     */
    void updateButtonStates();

    // --- 成员变量 ---
    Ui::NewStitcherDialog *ui;  // Qt Designer生成的UI类实例
    QStringList imagePaths;     // 存储用户选择的图像文件路径列表
    QPixmap resultPixmap;       // 存储最终拼接成功后的图像
};

#endif // NEWSTITCHERDIALOG_H
