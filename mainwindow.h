#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>

// --- 新增的头文件 ---
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
// --- 头文件结束 ---

// 向前声明，避免包含整个头文件
class QKeyEvent;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // 重写事件过滤器来处理 graphicsView 上的滚轮事件
    bool eventFilter(QObject *watched, QEvent *event) override;
    // 重写键盘按下事件来处理快捷键
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // 响应“打开”菜单动作的槽函数
    void on_actionopen_triggered();

private:
    // 缩放图像
    void scaleImage(double newScale);
    // 使图像适应窗口大小
    void fitToWindow();
    // 更新底部图片信息面板
    void updateImageInfo();

    Ui::MainWindow *ui;
    QPixmap originalPixmap;    // 存储原始加载的图像
    QString currentFilePath;   // 存储当前文件的路径
    double scaleFactor;        // 当前的缩放比例

    // --- 替换为 Graphics View 相关的成员 ---
    QGraphicsScene *imageScene;         // 场景，用于管理图像项
    QGraphicsPixmapItem *pixmapItem;    // 图像项，用于在场景中显示图像
    // --- 成员变量修改结束 ---
};
#endif // MAINWINDOW_H
