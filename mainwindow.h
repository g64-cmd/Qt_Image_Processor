#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

// --- 向前声明 ---
class QKeyEvent;
class QModelIndex;
class StagingAreaManager;
class DraggableItemModel;
class QUndoStack; // <-- 新增

class ProcessCommand; // <-- 新增

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    // --- 让 ProcessCommand 成为友元类，以便它能访问私有成员和函数 ---
    friend class ProcessCommand;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // “撤销/重做”的槽函数不再需要，我们将直接连接到 QUndoStack
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

    bool saveImageToFile(const QString &filePath);

    // --- 新增/修改的成员和函数 ---
    void updateImageFromCommand(const QString &imageId, const QPixmap &pixmap);
    QString getCurrentImageId() const;
    QPixmap getCurrentImagePixmap() const;

    Ui::MainWindow *ui;
    QString currentStagedImageId;
    QString currentSavePath;
    QString currentBaseName;
    double scaleFactor;

    QGraphicsScene *imageScene;
    QGraphicsPixmapItem *pixmapItem;

    QPixmap processedPixmap;

    StagingAreaManager *stagingManager;
    DraggableItemModel *stagingModel;
    QUndoStack *undoStack; // <-- 撤销/重做 栈
};
#endif // MAINWINDOW_H
