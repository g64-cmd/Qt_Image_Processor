#ifndef PROCESSCOMMAND_H
#define PROCESSCOMMAND_H

#include <QUndoCommand>
#include <QPixmap>

// 向前声明 MainWindow 以避免循环引用
class MainWindow;

/**
 * @brief 代表一个图像处理操作的命令
 *
 * 继承自 QUndoCommand，用于实现撤销/重做功能。
 */
class ProcessCommand : public QUndoCommand
{
public:
    // 定义了所有可能的操作类型
    enum Operation {
        Sharpen,
        Grayscale,
        Canny
    };

    explicit ProcessCommand(MainWindow *window, Operation op, QUndoCommand *parent = nullptr);

    // QUndoCommand 的核心虚函数
    void undo() override;
    void redo() override;

private:
    MainWindow *mainWindow; // 指向主窗口，用于操作其数据和UI
    Operation operation;    // 本次命令的操作类型

    QString imageId;        // 操作的目标图片ID
    QPixmap beforePixmap;   // 操作前的图片状态
    QPixmap afterPixmap;    // 操作后的图片状态
};

#endif // PROCESSCOMMAND_H
