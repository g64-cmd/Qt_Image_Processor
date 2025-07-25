// =============================================================================
//
// Copyright (C) 2025 g64-cmd
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// =============================================================================

#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

// =============================================================================
// File: videoprocessor.h
//
// Description:
// 该文件定义了 VideoProcessor 和 VideoDecoder 类，它们共同负责视频的
// 解码、播放控制和效果处理。VideoDecoder 在后台线程中解码音视频，
// VideoProcessor 则作为主线程的控制器，连接UI和解码器。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QObject>
#include <QPixmap>
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QAudioSink>
#include <opencv2/opencv.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>

// --- 前置声明 ---
class QStringListModel;
class QModelIndex;
class QIODevice;
class QTimer;
namespace Ui { class MainWindow; }

/**
 * @class VideoDecoder
 * @brief 在后台线程中解码音视频文件的类。
 *
 * [控制流程]
 * 1. 由主线程的 VideoProcessor 创建实例。
 * 2. 调用 startDecoding() 启动新线程并执行 run()。
 * 3. run() 方法中循环使用FFmpeg解码音视频，并将解码后的数据块放入两个线程安全的队列中。
 * 4. 主线程通过 getVideoFrame() 和 getAudioChunk() 从队列中取出数据。
 * 5. 通过 stop() 和 seek() 方法响应主线程的控制。
 */
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
    // 当 seek 操作在解码线程中完成后发射，通知主线程可以进行下一步操作（如重建音频设备）。
    void seekFinished();

protected:
    void run() override;

private:
    // --- 线程控制与状态变量 ---
    QString sourcePath; // 当前解码的文件路径
    // [关键变量] 线程停止标志。volatile 关键字确保多线程间的可见性。主线程设为true，run()循环会检测到并退出。
    volatile bool stopped = false;
    // [关键变量] 跳转请求时间点（毫秒）。主线程设置此值（非-1），run()循环检测到后会执行av_seek_frame。
    qint64 seekRequest = -1;

    // --- 数据结构 ---
    struct VideoFrame {
        cv::Mat frame;
        qint64 pts; // 视频帧的显示时间戳 (Presentation Timestamp)，单位：毫秒
    };

    // --- 线程安全的数据缓冲区 ---
    // [关键变量] 视频帧队列。解码线程作为生产者，主线程作为消费者。
    QQueue<VideoFrame> videoQueue;
    // [关键变量] 音频块队列。解码线程作为生产者，主线程作为消费者。
    QQueue<QByteArray> audioQueue;
    // [关键变量] 互斥锁，用于保护对 videoQueue 和 audioQueue 的访问，确保线程安全。
    QMutex mutex;

    // --- 视频元数据 ---
    double videoFPS = 0.0; // 视频的帧率
    qint64 durationMs = 0; // 视频总时长（毫秒）
};


/**
 * @class VideoProcessor
 * @brief 视频处理和播放的中心控制器（状态机）。
 *
 * [控制流程]
 * 1. 在主线程中创建，并与UI元素关联。
 * 2. 响应用户操作（如点击播放、拖动滑块），管理播放列表。
 * 3. 创建并控制 VideoDecoder 线程。
 * 4. 创建 QTimer (displayTimer) 作为播放“心跳”，定时触发 updateDisplay()。
 * 5. 创建 QAudioSink 用于播放音频。
 * 6. 在 updateDisplay() 中，实现音视频同步，应用效果，并通过信号更新UI。
 */
class VideoProcessor : public QObject
{
    Q_OBJECT

public:
    explicit VideoProcessor(Ui::MainWindow *ui, QObject *parent = nullptr);
    ~VideoProcessor();

public slots:
    // --- UI交互槽函数 ---
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
    // --- 内部逻辑槽函数 ---
    void updateDisplay();
    void onSeekFinished();
    void handleAudioStateChange(QAudio::State state);
    void recreateAudioSink();

signals:
    // --- 向外（MainWindow）通知的信号 ---
    void videoOpened(bool success, int totalDurationMs, double fps);
    void frameReady(const QPixmap &frame);
    void progressUpdated(const QString &timeString, int position, int duration);

private:
    // --- 辅助方法 ---
    cv::Mat applyEffects(const cv::Mat& frame);
    void updatePlayPauseButton(bool isPlaying);
    void loadFaceDetector();
    QString formatTime(qint64 ms);
    void stopCurrentVideo();

    // --- 核心组件 ---
    Ui::MainWindow *ui; // 指向UI对象，用于直接操作UI控件
    QStringListModel *videoListModel; // 视频播放列表的数据模型
    VideoDecoder* decoderThread = nullptr; // 指向后台解码线程的指针
    QTimer* displayTimer = nullptr; // 主播放定时器，驱动UI刷新
    QAudioSink* audioSink = nullptr; // Qt的音频播放组件
    QIODevice* audioDevice = nullptr; // 从AudioSink获取的用于写入音频数据的设备
    QAudioFormat audioFormat; // 音频播放的格式

    // --- 状态管理变量 ---
    // [关键变量] 标记当前是否处于播放状态。
    bool isVideoPlaying = false;
    // [关键变量] 在开始跳转前，记录当时的播放状态，以便跳转后恢复。
    bool wasPlayingBeforeSeek = false;
    // [关键变量] 标记当前是否正在进行跳转操作。这是一个状态锁，防止在跳转过程中执行其他冲突操作。
    bool isSeeking = false;

    // --- 数据变量 ---
    QPixmap currentPixmap; // 当前准备显示的视频帧
    qint64 videoDurationMs = 0; // 当前视频的总时长

    // --- 特效与录制 ---
    dlib::frontal_face_detector faceDetector; // dlib人脸检测器
    cv::VideoWriter videoWriter; // OpenCV视频写入器（用于录制）
    bool isRecording = false; // 标记是否正在录制
};

#endif // VIDEOPROCESSOR_H
