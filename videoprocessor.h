// videoprocessor.h
#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

#include <QObject>
#include <QPixmap>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>

class QTimer;
class QStringListModel;
class QModelIndex;
namespace Ui { class MainWindow; } // Forward declaration of the UI class

class VideoProcessor : public QObject
{
    Q_OBJECT
public:
    explicit VideoProcessor(Ui::MainWindow *ui, QObject *parent = nullptr);
    ~VideoProcessor();

public slots:
    // Slots to be connected from MainWindow's UI
    void addVideos();
    void removeSelectedVideo();
    void playVideoAtIndex(const QModelIndex &index);
    void togglePlayPause();
    void seek(int position);
    void stopSeeking();
    void setSpeed(int index);
    void saveCurrentFrame();
    void toggleRecording();

private slots:
    void processNextFrame();

signals:
    // --- FIX: Added the missing signal declaration ---
    void videoOpened(bool success, int totalFrames, double fps);
    void frameReady(const QPixmap &frame);
    void progressUpdated(const QString &timeString, int framePosition, int frameCount);

private:
    cv::Mat applyEffects(const cv::Mat& frame);
    void updatePlayPauseButton(bool isPlaying);
    void loadFaceCascade();

    Ui::MainWindow *ui; // Pointer to the UI elements
    QTimer *videoTimer;
    cv::VideoCapture videoCapture;
    QStringListModel *videoListModel;

    bool isVideoPlaying;
    bool isSliderBeingDragged;
    double currentPlaybackSpeed;

    cv::Mat currentFrame; // The original frame from video
    QPixmap currentPixmap; // The processed pixmap to be displayed

    cv::CascadeClassifier faceCascade;
    cv::VideoWriter videoWriter;
    bool isRecording;
};

#endif // VIDEOPROCESSOR_H
