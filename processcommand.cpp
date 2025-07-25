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
// File: processcommand.cpp
//
// Description:
// ProcessCommand 类的实现文件。该文件实现了命令的构造、撤销和重做逻辑。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "processcommand.h"
#include "mainwindow.h"
#include "imageprocessor.h"

/**
 * @brief ProcessCommand 构造函数。
 *
 * 在命令被创建时，它会立即从 MainWindow 获取当前正在处理的图像ID和
 * 图像内容（作为“操作前”的状态），并根据操作类型设置命令的描述文本。
 *
 * @param window 指向主窗口的指针。
 * @param op 要执行的操作类型。
 * @param parent 父命令。
 */
ProcessCommand::ProcessCommand(MainWindow *window, Operation op, QUndoCommand *parent)
    : QUndoCommand(parent), mainWindow(window), operation(op)
{
    // 记录操作前的状态
    imageId = mainWindow->getCurrentImageId();
    beforePixmap = mainWindow->getCurrentImagePixmap();

    // 根据操作类型设置在“编辑”菜单中显示的文本
    switch (operation) {
    case Sharpen:
        setText("图像锐化");
        break;
    case Grayscale:
        setText("灰度化");
        break;
    case Canny:
        setText("Canny 边缘检测");
        break;
    }
}

/**
 * @brief 撤销操作。
 *
 * 当用户选择“撤销”时，此函数被调用。它指示主窗口使用存储的
 * `beforePixmap` 来更新UI，恢复到操作之前的状态。
 */
void ProcessCommand::undo()
{
    mainWindow->updateImageFromCommand(imageId, beforePixmap);
}

/**
 * @brief 重做操作。
 *
 * 当命令第一次被推入 QUndoStack 或用户选择“重做”时，此函数被调用。
 *
 * 它采用“懒计算”的策略：
 * 1. 检查 `afterPixmap` 是否为空。如果为空，说明这是第一次执行 `redo`。
 * 2. 执行实际的图像处理，并将结果存储在 `afterPixmap` 中。
 * 3. 如果处理失败，则将命令标记为“过时” (obsolete)，使其从栈中移除。
 * 4. 如果 `afterPixmap` 已存在（非首次执行），则直接用它来更新UI。
 */
void ProcessCommand::redo()
{
    // 懒计算：仅在第一次执行redo时才真正处理图像
    if (afterPixmap.isNull()) {
        QImage resultImage;
        switch (operation) {
        case Sharpen:
            resultImage = ImageProcessor::sharpen(beforePixmap.toImage());
            break;
        case Grayscale:
            resultImage = ImageProcessor::grayscale(beforePixmap.toImage());
            break;
        case Canny:
            resultImage = ImageProcessor::canny(beforePixmap.toImage());
            break;
        }

        // 检查处理是否成功
        if (!resultImage.isNull()) {
            // 缓存处理结果
            afterPixmap = QPixmap::fromImage(resultImage);
        } else {
            // 如果处理失败，则此命令无效，应从撤销栈中移除
            setObsolete(true);
            return;
        }
    }

    // 使用“操作后”的图像更新主窗口
    mainWindow->updateImageFromCommand(imageId, afterPixmap);
}
