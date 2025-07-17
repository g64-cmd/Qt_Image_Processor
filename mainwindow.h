#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

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
    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void on_actionopen_triggered();
    // --- 新增槽函数，用于响应锐化按钮点击 ---
    void on_imageSharpenButton_clicked();

private:
    void scaleImage(double newScale);
    void fitToWindow();
    void updateImageInfo();
    // --- 新增函数，用于更新显示 ---
    void updateDisplayImage(const QPixmap &pixmap);

    Ui::MainWindow *ui;
    QString currentFilePath;
    double scaleFactor;

    QGraphicsScene *imageScene;
    QGraphicsPixmapItem *pixmapItem;

    // --- 新增/修改的成员变量 ---
    QPixmap originalPixmap;    // 存储原始加载的图像
    QPixmap processedPixmap;   // 存储当前处理后的图像
};
#endif // MAINWINDOW_H
