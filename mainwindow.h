//mainwindow.h
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
class QUndoStack;
class ProcessCommand;
class HistogramWidget;
class VideoProcessor; // Forward declare the new class

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class ProcessCommand;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // --- Image Processing Slots ---
    void on_actionopen_triggered();
    void on_actionsave_triggered();
    void on_actionsave_as_triggered();
    void on_actionexit_triggered();
    void on_imageSharpenButton_clicked();
    void on_imageGrayscaleButton_clicked();
    void on_cannyButton_clicked();
    void on_imageStitchButton_clicked();
    void on_imageBlendButton_clicked();
    void on_textureMigrationButton_clicked();
    void on_beautyButton_clicked();
    void on_imageNewStitchButton_clicked();
    void on_recentImageView_clicked(const QModelIndex &index);
    void onStagedImageDropped(const QString &imageId);
    void on_gamma_clicked();
    void on_gammaSlider_valueChanged(int value);
    void on_brightnessSlider_valueChanged(int value);
    void on_contrastSlider_valueChanged(int value);
    void on_saturationSlider_valueChanged(int value);
    void on_hueSlider_valueChanged(int value);
    void onMouseMovedOnImage(const QPointF &scenePos);
    void on_applyAdjustmentsButton_clicked();
    void on_deleteStagedImageButton_clicked();

    // --- Slots to receive signals from VideoProcessor ---
    void updateVideoFrame(const QPixmap &frame);
    void updateVideoProgress(const QString &timeString, int framePosition, int frameCount);
    void onVideoOpened(bool success, int totalFrames, double fps);

private:
    // --- Image Processing Methods ---
    void scaleImage(double newScale);
    void fitToWindow();
    void updateImageInfo();
    void updateDisplayImage(const QPixmap &pixmap);
    void loadNewImageFromFile(const QString &filePath);
    void displayImageFromStagingArea(const QString &imageId);
    bool saveImageToFile(const QString &filePath);
    void updateImageFromCommand(const QString &imageId, const QPixmap &pixmap);
    QString getCurrentImageId() const;
    QPixmap getCurrentImagePixmap() const;
    void applyAllAdjustments();
    void resetAdjustmentSliders();
    void updateExtraInfoPanels(const QPixmap &pixmap);
    void clearMainView();

    Ui::MainWindow *ui;

    // --- Image Tab Members ---
    QString currentStagedImageId;
    QString currentSavePath;
    QString currentBaseName;
    double scaleFactor;
    QGraphicsScene *imageScene;
    QGraphicsPixmapItem *pixmapItem;
    QPixmap processedPixmap;
    StagingAreaManager *stagingManager;
    DraggableItemModel *stagingModel;
    QUndoStack *undoStack;
    int currentBrightness;
    int currentContrast;
    int currentSaturation;
    int currentHue;

    // --- Video Tab Members ---
    VideoProcessor *videoProcessor;
    QGraphicsScene *videoScene;
    QGraphicsPixmapItem *videoPixmapItem;
};
#endif // MAINWINDOW_H
