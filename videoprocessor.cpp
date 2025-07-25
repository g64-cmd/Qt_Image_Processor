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

// =============================================================================
// File: videoprocessor.cpp
//
// Description:
// VideoProcessor 和 VideoDecoder 类的实现文件。这是整个视频播放功能
// 的核心，采用了典型的“生产者-消费者”多线程模型。
//
// [架构概览]
// 1. VideoDecoder (生产者): 运行在一个独立的后台线程。它使用FFmpeg库
//    来解码视频文件，将解码出的视频帧(cv::Mat)和音频块(QByteArray)
//    分别放入两个线程安全的队列中。
// 2. VideoProcessor (消费者/控制器): 运行在主GUI线程。它负责：
//    a. 响应用户的UI操作（播放、暂停、跳转等）。
//    b. 创建和管理VideoDecoder线程。
//    c. 创建一个QTimer作为“播放心跳”，定时从VideoDecoder的队列中取出数据。
//    d. 使用QAudioSink播放音频。
//    e. 以音频播放的进度为基准（音频时钟），从视频队列中取出最匹配的
//       一帧进行显示，从而实现音视频同步。
//    f. 在显示前，对视频帧应用各种视觉效果。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "videoprocessor.h"
#include "ui_mainwindow.h"
#include "imageconverter.h"
#include "imageprocessor.h"
#include <QStringListModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QTemporaryFile>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QTimer>
#include <algorithm> // For std::sort

// 包含 FFmpeg C语言头文件
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
}

// =============================================================================
// VideoDecoder Implementation (生产者线程)
// =============================================================================

/**
 * @brief VideoDecoder 构造函数。
 */
VideoDecoder::VideoDecoder(QObject* parent) : QThread(parent) {}

/**
 * @brief VideoDecoder 析构函数。
 *
 * 确保线程在对象销毁前被安全地停止和等待。这是一个良好实践，
 * 可以防止悬挂线程或资源泄漏。
 */
VideoDecoder::~VideoDecoder() {
    stop(); // 设置停止标志
    wait(); // 等待run()函数执行完毕
}

/**
 * @brief 启动解码过程。
 * @param filePath 要解码的视频文件路径。
 * @return 如果视频成功打开并获取到时长信息，则返回true。
 */
bool VideoDecoder::startDecoding(const QString &filePath) {
    // 如果上一个解码线程还在运行，先停止并等待它结束
    if (isRunning()) {
        stop();
        wait();
    }
    // 设置新的文件路径和状态
    sourcePath = filePath;
    stopped = false;
    seekRequest = -1;
    // 启动新线程，Qt会自动调用run()方法
    start();
    // 短暂等待，以确保FFmpeg有时间打开文件并获取时长。
    // 这是一个简单的同步机制，让调用者可以立即知道文件是否有效。
    msleep(200);
    return durationMs > 0;
}

/**
 * @brief 请求停止解码线程。
 *
 * 这是一个线程安全的异步停止请求。它只设置标志位，
 * run()循环会在下一次迭代时检查这个标志并自行退出。
 */
void VideoDecoder::stop() {
    stopped = true;
}

/**
 * @brief 请求跳转到指定时间点。
 *
 * 这是一个线程安全的异步跳转请求。它只设置请求的时间点，
 * run()循环会在下一次迭代时检查到并执行实际的跳转操作。
 * @param ms 目标时间点（毫秒）。
 */
void VideoDecoder::seek(qint64 ms) {
    seekRequest = ms;
}

/**
 * @brief 从视频队列中获取与当前音频时间戳最匹配的视频帧。
 *
 * 这是实现音视频同步的关键部分。它以音频播放进度为基准，
 * 从视频队列中找到时间上最接近的一帧。
 * @param audio_pts 当前音频播放的时间戳（毫秒）。
 * @return 匹配的视频帧 (cv::Mat)。
 */
cv::Mat VideoDecoder::getVideoFrame(qint64 audio_pts) {
    QMutexLocker locker(&mutex); // 锁定互斥锁，保护队列访问
    cv::Mat frame;
    // 循环丢弃所有时间戳小于等于当前音频时间戳的“过时”视频帧。
    // 这确保了视频不会落后于音频。
    while (!videoQueue.isEmpty() && videoQueue.head().pts <= audio_pts) {
        frame = videoQueue.dequeue().frame; // 取出帧
        // 优化：如果取出当前帧后，下一帧的时间戳已经超前于音频，
        // 那么当前帧就是最完美的匹配，无需再继续寻找。
        if (!videoQueue.isEmpty() && videoQueue.head().pts > audio_pts) break;
    }
    return frame; // 返回找到的最佳匹配帧，或者最后一帧
}

/**
 * @brief 从音频队列中获取一个音频块。
 * @return 音频数据块 (QByteArray)。
 */
QByteArray VideoDecoder::getAudioChunk() {
    QMutexLocker locker(&mutex); // 锁定互斥锁，保护队列访问
    if (audioQueue.isEmpty()) return QByteArray();
    return audioQueue.dequeue();
}

/**
 * @brief 解码线程的主函数。这是在新线程中执行的所有代码。
 *
 * 使用 FFmpeg 库循环读取、解码音视频包，并存入队列。
 */
void VideoDecoder::run() {
    // --- 1. 初始化 FFmpeg ---
    AVFormatContext* formatCtx = nullptr;
    // 打开输入文件并读取头信息
    if (avformat_open_input(&formatCtx, sourcePath.toLocal8Bit().constData(), nullptr, nullptr) != 0) {
        qWarning() << "FFmpeg: 无法打开文件" << sourcePath; return;
    }
    // 查找流信息
    if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
        qWarning() << "FFmpeg: 无法找到流信息"; avformat_close_input(&formatCtx); return;
    }
    // 获取视频总时长（毫秒）
    durationMs = formatCtx->duration / (AV_TIME_BASE / 1000);

    // --- 2. 查找并打开音视频解码器 ---
    int videoStreamIndex = av_find_best_stream(formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    int audioStreamIndex = av_find_best_stream(formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (videoStreamIndex < 0 || audioStreamIndex < 0) {
        qWarning() << "FFmpeg: 无法同时找到视频流和音频流。"; avformat_close_input(&formatCtx); return;
    }

    // a. 准备视频解码器
    AVCodecParameters* videoCodecParams = formatCtx->streams[videoStreamIndex]->codecpar;
    const AVCodec* videoCodec = avcodec_find_decoder(videoCodecParams->codec_id);
    AVCodecContext* videoCodecCtx = avcodec_alloc_context3(videoCodec);
    avcodec_parameters_to_context(videoCodecCtx, videoCodecParams);
    if (avcodec_open2(videoCodecCtx, videoCodec, nullptr) < 0) { qWarning() << "FFmpeg: 无法打开视频解码器"; avformat_close_input(&formatCtx); return; }
    videoFPS = av_q2d(formatCtx->streams[videoStreamIndex]->r_frame_rate);
    if (videoFPS <= 0) videoFPS = 25; // 提供一个备用FPS

    // b. 准备音频解码器
    AVCodecParameters* audioCodecParams = formatCtx->streams[audioStreamIndex]->codecpar;
    const AVCodec* audioCodec = avcodec_find_decoder(audioCodecParams->codec_id);
    AVCodecContext* audioCodecCtx = avcodec_alloc_context3(audioCodec);
    avcodec_parameters_to_context(audioCodecCtx, audioCodecParams);
    if(avcodec_open2(audioCodecCtx, audioCodec, nullptr) < 0){ qWarning() << "FFmpeg: 无法打开音频解码器"; avformat_close_input(&formatCtx); return; }

    // --- 3. 设置格式转换器 ---
    // a. 音频重采样：将源音频格式转换为Qt AudioSink支持的格式（立体声, 16位有符号整数, 48kHz）
    SwrContext* swrCtx = nullptr;
    AVChannelLayout out_ch_layout = AV_CHANNEL_LAYOUT_STEREO;
    swr_alloc_set_opts2(&swrCtx, &out_ch_layout, AV_SAMPLE_FMT_S16, 48000, &audioCodecCtx->ch_layout, audioCodecCtx->sample_fmt, audioCodecCtx->sample_rate, 0, nullptr);
    swr_init(swrCtx);
    // b. 视频缩放/格式转换：将源视频像素格式转换为OpenCV Mat支持的BGR24格式
    SwsContext* swsCtx = sws_getContext(videoCodecCtx->width, videoCodecCtx->height, videoCodecCtx->pix_fmt, videoCodecCtx->width, videoCodecCtx->height, AV_PIX_FMT_BGR24, SWS_BILINEAR, nullptr, nullptr, nullptr);

    // 分配用于循环的数据包和帧
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    // --- 4. 主解码循环 ---
    while (!stopped) {
        // a. 处理跳转请求
        if (seekRequest != -1) {
            // 将毫秒时间转换为FFmpeg内部的时间基（timestamp）
            qint64 seek_target_ts = seekRequest * formatCtx->streams[videoStreamIndex]->time_base.den / (1000 * formatCtx->streams[videoStreamIndex]->time_base.num);
            // 执行跳转
            av_seek_frame(formatCtx, -1, seek_target_ts, AVSEEK_FLAG_BACKWARD);
            // 清空解码器内部的缓冲区
            avcodec_flush_buffers(videoCodecCtx);
            avcodec_flush_buffers(audioCodecCtx);
            // 清空我们自己的队列
            mutex.lock();
            videoQueue.clear(); audioQueue.clear();
            seekRequest = -1; // 重置请求
            mutex.unlock();
            // 通知主线程跳转已完成
            emit seekFinished();
        }

        // b. 控制队列大小 (背压)，防止队列过大消耗过多内存
        if (videoQueue.size() > 100 || audioQueue.size() > 200) { msleep(10); continue; }

        // c. 从文件中读取一个数据包 (packet)
        if (av_read_frame(formatCtx, packet) < 0) { stopped = true; break; } // 文件读完或出错

        // d. 解码视频包
        if (packet->stream_index == videoStreamIndex) {
            if (avcodec_send_packet(videoCodecCtx, packet) == 0) {
                while (avcodec_receive_frame(videoCodecCtx, frame) == 0) {
                    cv::Mat cvFrame(videoCodecCtx->height, videoCodecCtx->width, CV_8UC3);
                    uint8_t* dest[] = { cvFrame.data };
                    int dest_linesize[] = { (int)cvFrame.step };
                    // 转换像素格式
                    sws_scale(swsCtx, frame->data, frame->linesize, 0, videoCodecCtx->height, dest, dest_linesize);
                    VideoFrame vf;
                    vf.frame = cvFrame.clone();
                    // 计算以毫秒为单位的显示时间戳
                    vf.pts = frame->pts * 1000 * av_q2d(formatCtx->streams[videoStreamIndex]->time_base);
                    QMutexLocker locker(&mutex);
                    videoQueue.enqueue(vf);
                }
            }
            // e. 解码音频包
        } else if (packet->stream_index == audioStreamIndex) {
            if (avcodec_send_packet(audioCodecCtx, packet) == 0) {
                while (avcodec_receive_frame(audioCodecCtx, frame) == 0) {
                    uint8_t* resampled_data = nullptr;
                    // 计算重采样后需要的缓冲区大小
                    int out_samples = av_rescale_rnd(swr_get_delay(swrCtx, frame->sample_rate) + frame->nb_samples, 48000, frame->sample_rate, AV_ROUND_UP);
                    av_samples_alloc(&resampled_data, NULL, 2, out_samples, AV_SAMPLE_FMT_S16, 0);
                    // 执行重采样
                    out_samples = swr_convert(swrCtx, &resampled_data, out_samples, (const uint8_t**)frame->data, frame->nb_samples);
                    int data_size = out_samples * 2 * 2; // 采样数 * 通道数 * 采样大小(16bit=2bytes)
                    QByteArray audioData((char*)resampled_data, data_size);
                    av_freep(&resampled_data);
                    QMutexLocker locker(&mutex);
                    audioQueue.enqueue(audioData);
                }
            }
        }
        av_packet_unref(packet); // 释放数据包，准备下一次循环
    }

    // --- 5. 清理所有FFmpeg资源 ---
    av_packet_free(&packet); av_frame_free(&frame); sws_freeContext(swsCtx); swr_free(&swrCtx);
    avcodec_free_context(&videoCodecCtx); avcodec_free_context(&audioCodecCtx); avformat_close_input(&formatCtx);
    qDebug() << "解码线程已结束。";
}


// =============================================================================
// VideoProcessor Implementation (消费者/控制器)
// =============================================================================

VideoProcessor::VideoProcessor(Ui::MainWindow *ui, QObject *parent)
    : QObject(parent), ui(ui)
{
    videoListModel = new QStringListModel(this);
    ui->videoListView->setModel(videoListModel);
    ui->speedComboBox->addItems({"0.5x", "1.0x", "1.5x", "2.0x"});
    ui->speedComboBox->setCurrentIndex(1);
    ui->filterComboBox->addItems({"无", "模糊", "锐化"});
    loadFaceDetector();
    ui->controlBar->setEnabled(false);
    ui->videoEffectsToolBox->setEnabled(false);
}

VideoProcessor::~VideoProcessor() {
    stopCurrentVideo();
}

void VideoProcessor::stopCurrentVideo() {
    if (displayTimer) {
        displayTimer->stop();
        delete displayTimer; displayTimer = nullptr;
    }
    if (decoderThread) {
        decoderThread->stop();
        decoderThread->wait(1000); // 等待最多1秒
        delete decoderThread; decoderThread = nullptr;
    }
    if (audioSink) {
        disconnect(audioSink, &QAudioSink::stateChanged, this, &VideoProcessor::handleAudioStateChange);
        audioSink->stop();
        delete audioSink; audioSink = nullptr;
        audioDevice = nullptr;
    }
}

void VideoProcessor::loadFaceDetector() {
    try {
        faceDetector = dlib::get_frontal_face_detector();
        qDebug() << "dlib frontal face detector loaded successfully.";
    } catch (const std::exception& e) {
        qWarning() << "Failed to load dlib face detector:" << e.what();
    }
}

void VideoProcessor::addVideos() {
    QStringList files = QFileDialog::getOpenFileNames(qobject_cast<QWidget*>(parent()), "选择视频文件", "", "Video Files (*.mp4 *.avi *.mov *.mkv)");
    if (!files.isEmpty()) {
        int row = videoListModel->rowCount();
        videoListModel->insertRows(row, files.count());
        for (int i = 0; i < files.count(); ++i) {
            QModelIndex index = videoListModel->index(row + i, 0);
            videoListModel->setData(index, files[i]);
        }
    }
}

void VideoProcessor::removeSelectedVideo() {
    QModelIndexList selected = ui->videoListView->selectionModel()->selectedIndexes();
    std::sort(selected.begin(), selected.end(), std::greater<QModelIndex>());
    for (const QModelIndex& index : selected) {
        videoListModel->removeRow(index.row());
    }
}

void VideoProcessor::playVideoAtIndex(const QModelIndex &index)
{
    stopCurrentVideo(); // 播放新视频前，先停止并清理上一个
    QString filePath = videoListModel->data(index, Qt::DisplayRole).toString();

    decoderThread = new VideoDecoder(this);
    connect(decoderThread, &VideoDecoder::seekFinished, this, &VideoProcessor::onSeekFinished);

    if (!decoderThread->startDecoding(filePath)) {
        QMessageBox::critical(qobject_cast<QWidget*>(parent()), "错误", "无法打开或解析视频文件。");
        stopCurrentVideo(); return;
    }

    videoDurationMs = decoderThread->getDurationMs();
    double fps = decoderThread->getFPS();

    // 设置音频输出格式
    audioFormat.setSampleRate(48000);
    audioFormat.setChannelCount(2);
    audioFormat.setSampleFormat(QAudioFormat::Int16);
    recreateAudioSink(); // 创建音频播放实例

    // 创建并启动显示定时器
    displayTimer = new QTimer(this);
    connect(displayTimer, &QTimer::timeout, this, &VideoProcessor::updateDisplay);
    displayTimer->start(1000.0 / fps);

    // 更新状态
    isVideoPlaying = true;
    emit videoOpened(true, videoDurationMs, fps);
    ui->controlBar->setEnabled(true);
    ui->videoEffectsToolBox->setEnabled(true);
    updatePlayPauseButton(true);
}

void VideoProcessor::togglePlayPause() {
    if (!decoderThread || isSeeking) return; // 跳转时不允许操作
    isVideoPlaying = !isVideoPlaying;
    if (isVideoPlaying) {
        displayTimer->start();
        if(audioSink) audioSink->resume();
    } else {
        displayTimer->stop();
        if(audioSink) audioSink->suspend();
    }
    updatePlayPauseButton(isVideoPlaying);
}

void VideoProcessor::updateDisplay() {
    if (!decoderThread || !isVideoPlaying || isSeeking) return;

    // [音视频同步-步骤1] 填充音频缓冲区
    while(audioDevice && audioSink && audioSink->state() != QAudio::StoppedState && audioSink->bytesFree() > 0) {
        QByteArray audioData = decoderThread->getAudioChunk();
        if (audioData.isEmpty()) break;
        audioDevice->write(audioData);
    }

    // [音视频同步-步骤2] 获取音频时钟
    qint64 audioPts = audioSink ? (audioSink->processedUSecs() / 1000) : 0;

    // [音视频同步-步骤3] 获取对应的视频帧
    cv::Mat frame = decoderThread->getVideoFrame(audioPts);
    if (frame.empty()) return;

    // [音视频同步-步骤4] 处理并显示
    cv::Mat processedFrame = applyEffects(frame);
    currentPixmap = QPixmap::fromImage(ImageConverter::matToQImage(processedFrame));
    emit frameReady(currentPixmap);
    emit progressUpdated(QString("%1 / %2").arg(formatTime(audioPts)).arg(formatTime(videoDurationMs)), audioPts, videoDurationMs);
}

void VideoProcessor::onSliderPressed() {
    if (!decoderThread) return;
    // [跳转流程-步骤1]
    wasPlayingBeforeSeek = isVideoPlaying;
    if (isVideoPlaying) {
        displayTimer->stop();
        if(audioSink) audioSink->suspend();
    }
}

void VideoProcessor::seek(int position) {
    if (!decoderThread) return;
    emit progressUpdated(QString("%1 / %2").arg(formatTime(position)).arg(formatTime(videoDurationMs)), position, videoDurationMs);
}

void VideoProcessor::stopSeeking() {
    if (!decoderThread) return;
    // [跳转流程-步骤2]
    isSeeking = true;
    qDebug() << "Seek requested to:" << ui->videoSlider->value() << "ms. Notifying decoder thread.";
    decoderThread->seek(ui->videoSlider->value());
}

void VideoProcessor::onSeekFinished() {
    // [跳转流程-步骤3]
    qDebug() << "Decoder has finished seek. Now stopping audio sink...";
    if (audioSink) {
        audioSink->stop();
    }
}

void VideoProcessor::handleAudioStateChange(QAudio::State state) {
    qDebug() << "Audio state changed to:" << state;
    // [跳转流程-步骤4]
    if (isSeeking && state == QAudio::StoppedState) {
        qDebug() << "Audio sink confirmed stopped. Forcing recreation.";
        recreateAudioSink();
    }
}

void VideoProcessor::recreateAudioSink() {
    // [跳转流程-步骤5]
    qDebug() << "Recreating AudioSink instance...";
    if (audioSink) {
        disconnect(audioSink, &QAudioSink::stateChanged, this, &VideoProcessor::handleAudioStateChange);
        delete audioSink;
        audioSink = nullptr;
    }
    const QAudioDevice &defaultAudioDevice = QMediaDevices::defaultAudioOutput();
    if (defaultAudioDevice.isNull()) { qCritical() << "Cannot recreate audio sink, no default device found."; isSeeking = false; return; }

    audioSink = new QAudioSink(defaultAudioDevice, audioFormat, this);
    connect(audioSink, &QAudioSink::stateChanged, this, &VideoProcessor::handleAudioStateChange);
    audioDevice = audioSink->start();
    if (!audioDevice) { qCritical() << "Recreated audio sink FAILED to start. Error:" << audioSink->error(); isSeeking = false; return; }
    qDebug() << "Recreated audio sink started successfully.";

    // [跳转流程-步骤6]
    if (isSeeking) {
        if (wasPlayingBeforeSeek) {
            if(audioSink) audioSink->resume();
            if(displayTimer) displayTimer->start();
        } else {
            cv::Mat frame = decoderThread->getVideoFrame(ui->videoSlider->value());
            if (!frame.empty()) {
                cv::Mat processedFrame = applyEffects(frame);
                currentPixmap = QPixmap::fromImage(ImageConverter::matToQImage(processedFrame));
                emit frameReady(currentPixmap);
            }
        }
        updatePlayPauseButton(wasPlayingBeforeSeek);
        isSeeking = false;
    }
}

void VideoProcessor::setSpeed(int index) {
    qreal rate = 1.0;
    switch (index) {
    case 0: rate = 0.5; break;
    case 1: rate = 1.0; break;
    case 2: rate = 1.5; break;
    case 3: rate = 2.0; break;
    }
    if(displayTimer && decoderThread && decoderThread->getFPS() > 0) {
        displayTimer->setInterval(1000.0 / (decoderThread->getFPS() * rate));
    }
}

void VideoProcessor::saveCurrentFrame() {
    if (currentPixmap.isNull()) {
        QMessageBox::warning(qobject_cast<QWidget*>(parent()), "无内容", "没有可保存的视频帧。");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(qobject_cast<QWidget*>(parent()), "保存当前帧", "", "PNG Image (*.png)");
    if (!fileName.isEmpty()) {
        currentPixmap.save(fileName);
    }
}

void VideoProcessor::toggleRecording() {
    QMessageBox::information(qobject_cast<QWidget*>(parent()), "待实现", "录制功能将在下一步中实现。");
    ui->recordButton->setChecked(false);
}

cv::Mat VideoProcessor::applyEffects(const cv::Mat &frame) {
    cv::Mat result = frame.clone();
    int b = ui->videoBrightnessSlider->value(); int c = ui->videoContrastSlider->value();
    int s = ui->videoSaturationSlider->value(); int h = ui->videoHueSlider->value();

    if (b != 0 || c != 0 || s != 0 || h != 0) {
        QImage tempQImg = ImageConverter::matToQImage(result);
        tempQImg = ImageProcessor::adjustColor(tempQImg, b, c, s, h);
        result = ImageConverter::qImageToMat(tempQImg);
        if (result.channels() == 4) cv::cvtColor(result, result, cv::COLOR_BGRA2BGR);
    }
    if (ui->grayscaleCheckBox->isChecked()) {
        cv::cvtColor(result, result, cv::COLOR_BGR2GRAY);
        cv::cvtColor(result, result, cv::COLOR_GRAY2BGR);
    }
    if (ui->faceDetectCheckBox->isChecked()) {
        try {
            dlib::cv_image<dlib::bgr_pixel> dlib_img(result);
            std::vector<dlib::rectangle> faces = faceDetector(dlib_img);
            for (const auto& face : faces) {
                cv::Rect rect(face.left(), face.top(), face.width(), face.height());
                cv::rectangle(result, rect, cv::Scalar(0, 255, 0), 2);
            }
        } catch (const std::exception& e) {
            qWarning() << "dlib face detection failed:" << e.what();
        }
    }
    return result;
}

void VideoProcessor::updatePlayPauseButton(bool playing) {
    isVideoPlaying = playing;
    if (isVideoPlaying) {
        ui->playPauseButton->setText("暂停");
        ui->playPauseButton->setIcon(QIcon(":/icons/resources/icons/pause.svg"));
    } else {
        ui->playPauseButton->setText("播放");
        ui->playPauseButton->setIcon(QIcon(":/icons/resources/icons/play.svg"));
    }
}

QString VideoProcessor::formatTime(qint64 ms) {
    int seconds = ms / 1000;
    return QString("%1:%2").arg(seconds / 60, 2, 10, QChar('0')).arg(seconds % 60, 2, 10, QChar('0'));
}
