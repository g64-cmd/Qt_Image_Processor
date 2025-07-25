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

#ifndef PROCESSCOMMAND_H
#define PROCESSCOMMAND_H

// =============================================================================
// File: processcommand.h
//
// Description:
// 该文件定义了 ProcessCommand 类，它实现了命令设计模式，用于封装
// 一个可撤销/重做的图像处理操作。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QUndoCommand>
#include <QPixmap>
#include <QString> // 包含 QString 的定义

// --- 前置声明 ---
class MainWindow;

/**
 * @class ProcessCommand
 * @brief 代表一个图像处理操作的命令。
 *
 * 继承自 QUndoCommand，用于实现撤销/重做功能。每个ProcessCommand
 * 实例都存储了操作前后的图像状态，以便在 QUndoStack 中进行切换。
 */
class ProcessCommand : public QUndoCommand
{
public:
    /**
     * @enum Operation
     * @brief 定义了此类可以封装的图像处理操作类型。
     */
    enum Operation {
        Sharpen,    // 锐化
        Grayscale,  // 灰度化
        Canny       // Canny边缘检测
    };

    /**
     * @brief 构造函数。
     *
     * 在构造时，会记录下操作前的图像状态。
     * @param window 指向主窗口的指针，命令需要通过它来获取和更新图像。
     * @param op 要执行的操作类型。
     * @param parent 父命令，默认为nullptr。
     */
    explicit ProcessCommand(MainWindow *window, Operation op, QUndoCommand *parent = nullptr);

    /**
     * @brief 撤销操作。
     *
     * QUndoStack 调用此函数时，它会将主窗口的图像恢复到操作前的状态。
     */
    void undo() override;

    /**
     * @brief 重做操作。
     *
     * QUndoStack 调用此函数时，它会执行图像处理，并将主窗口的图像
     * 更新到操作后的状态。首次推入栈时，此函数也会被自动调用。
     */
    void redo() override;

private:
    // --- 成员变量 ---
    MainWindow *mainWindow; // 指向主窗口，用于UI和数据交互
    Operation operation;    // 此命令代表的操作类型
    QString imageId;        // 被操作图像的唯一ID
    QPixmap beforePixmap;   // 操作前的图像快照
    QPixmap afterPixmap;    // 操作后的图像快照
};

#endif // PROCESSCOMMAND_H
