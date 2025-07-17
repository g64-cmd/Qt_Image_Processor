#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

class QKeyEvent;
class QModelIndex;
class StagingAreaManager;
class DraggableItemModel;

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
    // --- 新增/修改的槽函数 ---
    void on_actionopen_triggered();
    void on_actionsave_triggered();
    void on_actionsave_as_triggered();
    void on_actionexit_triggered();

    void on_imageSharpenButton_clicked();
    void on_imageGrayscaleButton_clicked();
    void on_cannyButton_clicked();
    void on_recentImageView_clicked(const QModelIndex &index);
    void onStagedImageDropped(const QString &imageId);

private:
    void scaleImage(double newScale);
    void fitToWindow();
    void updateImageInfo();
    void updateDisplayImage(const QPixmap &pixmap);

    void loadNewImageFromFile(const QString &filePath);
    void displayImageFromStagingArea(const QString &imageId);

    // --- 新增的保存辅助函数 ---
    bool saveImageToFile(const QString &filePath);

    Ui::MainWindow *ui;
    QString currentStagedImageId;
    QString currentSavePath; // <-- 新增：追踪当前图片的保存路径
    QString currentBaseName;
    double scaleFactor;

    QGraphicsScene *imageScene;
    QGraphicsPixmapItem *pixmapItem;

    QPixmap processedPixmap;

    StagingAreaManager *stagingManager;
    DraggableItemModel *stagingModel;
};
#endif // MAINWINDOW_H
