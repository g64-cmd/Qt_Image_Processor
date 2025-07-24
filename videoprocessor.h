#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

#include <QObject>
#include <QPixmap>
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QAudioSink>
#include <opencv2/opencv.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>

class QStringListModel;
class QModelIndex;
class QIODevice;
class QTimer;
namespace Ui { class MainWindow; }

class VideoDecoder : public QThread {
    Q_OBJECT
public:
    explicit VideoDecoder(QObject* parent = nullptr);
    ~VideoDecoder();
    bool startDecoding(const QString& filePath);
    void stop();
    void seek(qint64 ms);
    cv::Mat getVideoFrame(qint64 audio_pts);
    QByteArray getAudioChunk();
    double getFPS() const { return videoFPS; }
    qint64 getDurationMs() const { return durationMs; }

signals:
    void seekFinished();

protected:
    void run() override;

private:
    QString sourcePath;
    volatile bool stopped = false;
    qint64 seekRequest = -1;

    struct VideoFrame {
        cv::Mat frame;
        qint64 pts;
    };

    QQueue<VideoFrame> videoQueue;
    QQueue<QByteArray> audioQueue;
    QMutex mutex;
    double videoFPS = 0.0;
    qint64 durationMs = 0;
};


class VideoProcessor : public QObject
{
    Q_OBJECT

public:
    explicit VideoProcessor(Ui::MainWindow *ui, QObject *parent = nullptr);
    ~VideoProcessor();

public slots:
    void addVideos();
    void removeSelectedVideo();
    void playVideoAtIndex(const QModelIndex &index);
    void togglePlayPause();
    void onSliderPressed();
    void seek(int position);
    void stopSeeking();
    void setSpeed(int index);
    void saveCurrentFrame();
    void toggleRecording();

private slots:
    void updateDisplay();
    void onSeekFinished();
    void handleAudioStateChange(QAudio::State state);
    void recreateAudioSink();

signals:
    void videoOpened(bool success, int totalDurationMs, double fps);
    void frameReady(const QPixmap &frame);
    void progressUpdated(const QString &timeString, int position, int duration);

private:
    cv::Mat applyEffects(const cv::Mat& frame);
    void updatePlayPauseButton(bool isPlaying);
    void loadFaceDetector();
    QString formatTime(qint64 ms);
    void stopCurrentVideo();
    Ui::MainWindow *ui;
    QStringListModel *videoListModel;
    VideoDecoder* decoderThread = nullptr;
    QTimer* displayTimer = nullptr;
    QAudioSink* audioSink = nullptr;
    QIODevice* audioDevice = nullptr;
    QAudioFormat audioFormat;
    bool isVideoPlaying = false;
    bool wasPlayingBeforeSeek = false;
    bool isSeeking = false;
    QPixmap currentPixmap;
    qint64 videoDurationMs = 0;
    dlib::frontal_face_detector faceDetector;
    cv::VideoWriter videoWriter;
    bool isRecording = false;
};

#endif // VIDEOPROCESSOR_H
