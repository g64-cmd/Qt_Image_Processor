#ifndef PROCESSCOMMAND_H
#define PROCESSCOMMAND_H

#include <QUndoCommand>
#include <QPixmap>

class MainWindow;

/**
 * @brief 代表一个图像处理操作的命令
 *
 * 继承自 QUndoCommand，用于实现撤销/重做功能。
 */
class ProcessCommand : public QUndoCommand
{
public:
    enum Operation {
        Sharpen,
        Grayscale,
        Canny
    };

    explicit ProcessCommand(MainWindow *window, Operation op, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    MainWindow *mainWindow;
    Operation operation;
    QString imageId;
    QPixmap beforePixmap;
    QPixmap afterPixmap;
};

#endif // PROCESSCOMMAND_H
