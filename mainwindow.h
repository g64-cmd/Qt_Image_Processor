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
    void on_actionopen_triggered();
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

    // 加载逻辑重构
    void loadNewImageFromFile(const QString &filePath);
    void displayImageFromStagingArea(const QString &imageId);

    Ui::MainWindow *ui;
    QString currentStagedImageId; // <-- 关键：追踪当前正在显示的图片ID
    QString currentBaseName;
    double scaleFactor;

    QGraphicsScene *imageScene;
    QGraphicsPixmapItem *pixmapItem;

    QPixmap processedPixmap; // 只代表当前画布上的图片

    StagingAreaManager *stagingManager;
    DraggableItemModel *stagingModel;
};
#endif // MAINWINDOW_H
