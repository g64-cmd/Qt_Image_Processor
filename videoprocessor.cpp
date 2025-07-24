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

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
}

VideoDecoder::VideoDecoder(QObject* parent) : QThread(parent) {}
VideoDecoder::~VideoDecoder() { stop(); wait(); }

bool VideoDecoder::startDecoding(const QString &filePath) {
    if (isRunning()) { stop(); wait(); }
    sourcePath = filePath; stopped = false; seekRequest = -1;
    start(); msleep(200); return durationMs > 0;
}

void VideoDecoder::stop() { stopped = true; }
void VideoDecoder::seek(qint64 ms) { seekRequest = ms; }

cv::Mat VideoDecoder::getVideoFrame(qint64 audio_pts) {
    QMutexLocker locker(&mutex);
    cv::Mat frame;
    while (!videoQueue.isEmpty() && videoQueue.head().pts <= audio_pts) {
        frame = videoQueue.dequeue().frame;
        if (!videoQueue.isEmpty() && videoQueue.head().pts > audio_pts) break;
    }
    return frame;
}

QByteArray VideoDecoder::getAudioChunk() {
    QMutexLocker locker(&mutex);
    if (audioQueue.isEmpty()) return QByteArray();
    return audioQueue.dequeue();
}

void VideoDecoder::run() {
    AVFormatContext* formatCtx = nullptr;
    if (avformat_open_input(&formatCtx, sourcePath.toLocal8Bit().constData(), nullptr, nullptr) != 0) {
        qWarning() << "FFmpeg: 无法打开文件" << sourcePath; return;
    }
    if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
        qWarning() << "FFmpeg: 无法找到流信息"; avformat_close_input(&formatCtx); return;
    }
    durationMs = formatCtx->duration / (AV_TIME_BASE / 1000);
    int videoStreamIndex = av_find_best_stream(formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    int audioStreamIndex = av_find_best_stream(formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (videoStreamIndex < 0 || audioStreamIndex < 0) {
        qWarning() << "FFmpeg: 无法同时找到视频流和音频流。"; avformat_close_input(&formatCtx); return;
    }
    AVCodecParameters* videoCodecParams = formatCtx->streams[videoStreamIndex]->codecpar;
    const AVCodec* videoCodec = avcodec_find_decoder(videoCodecParams->codec_id);
    AVCodecContext* videoCodecCtx = avcodec_alloc_context3(videoCodec);
    avcodec_parameters_to_context(videoCodecCtx, videoCodecParams);
    if (avcodec_open2(videoCodecCtx, videoCodec, nullptr) < 0) {
        qWarning() << "FFmpeg: 无法打开视频解码器"; avformat_close_input(&formatCtx); return;
    }
    videoFPS = av_q2d(formatCtx->streams[videoStreamIndex]->r_frame_rate);
    if (videoFPS <= 0) videoFPS = 25;
    AVCodecParameters* audioCodecParams = formatCtx->streams[audioStreamIndex]->codecpar;
    const AVCodec* audioCodec = avcodec_find_decoder(audioCodecParams->codec_id);
    AVCodecContext* audioCodecCtx = avcodec_alloc_context3(audioCodec);
    avcodec_parameters_to_context(audioCodecCtx, audioCodecParams);
    if(avcodec_open2(audioCodecCtx, audioCodec, nullptr) < 0){
        qWarning() << "FFmpeg: 无法打开音频解码器"; avformat_close_input(&formatCtx); return;
    }
    SwrContext* swrCtx = nullptr;
    AVChannelLayout out_ch_layout = AV_CHANNEL_LAYOUT_STEREO;
    swr_alloc_set_opts2(&swrCtx, &out_ch_layout, AV_SAMPLE_FMT_S16, 48000, &audioCodecCtx->ch_layout, audioCodecCtx->sample_fmt, audioCodecCtx->sample_rate, 0, nullptr);
    swr_init(swrCtx);
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    SwsContext* swsCtx = sws_getContext(videoCodecCtx->width, videoCodecCtx->height, videoCodecCtx->pix_fmt, videoCodecCtx->width, videoCodecCtx->height, AV_PIX_FMT_BGR24, SWS_BILINEAR, nullptr, nullptr, nullptr);
    while (!stopped) {
        if (seekRequest != -1) {
            qint64 seek_target_ts = seekRequest * formatCtx->streams[videoStreamIndex]->time_base.den / (1000 * formatCtx->streams[videoStreamIndex]->time_base.num);
            if (av_seek_frame(formatCtx, -1, seek_target_ts, AVSEEK_FLAG_BACKWARD) < 0) {
                qWarning() << "FFmpeg: Seek failed";
            }
            avcodec_flush_buffers(videoCodecCtx);
            avcodec_flush_buffers(audioCodecCtx);
            mutex.lock();
            videoQueue.clear(); audioQueue.clear();
            seekRequest = -1;
            mutex.unlock();
            emit seekFinished();
        }
        if (videoQueue.size() > 100 || audioQueue.size() > 200) { msleep(10); continue; }
        if (av_read_frame(formatCtx, packet) < 0) { stopped = true; break; }
        if (packet->stream_index == videoStreamIndex) {
            if (avcodec_send_packet(videoCodecCtx, packet) == 0) {
                while (avcodec_receive_frame(videoCodecCtx, frame) == 0) {
                    cv::Mat cvFrame(videoCodecCtx->height, videoCodecCtx->width, CV_8UC3);
                    uint8_t* dest[] = { cvFrame.data };
                    int dest_linesize[] = { (int)cvFrame.step };
                    sws_scale(swsCtx, frame->data, frame->linesize, 0, videoCodecCtx->height, dest, dest_linesize);
                    VideoFrame vf;
                    vf.frame = cvFrame.clone();
                    vf.pts = frame->pts * 1000 * av_q2d(formatCtx->streams[videoStreamIndex]->time_base);
                    QMutexLocker locker(&mutex);
                    videoQueue.enqueue(vf);
                }
            }
        } else if (packet->stream_index == audioStreamIndex) {
            if (avcodec_send_packet(audioCodecCtx, packet) == 0) {
                while (avcodec_receive_frame(audioCodecCtx, frame) == 0) {
                    uint8_t* resampled_data = nullptr;
                    int out_samples = av_rescale_rnd(swr_get_delay(swrCtx, frame->sample_rate) + frame->nb_samples, 48000, frame->sample_rate, AV_ROUND_UP);
                    av_samples_alloc(&resampled_data, NULL, 2, out_samples, AV_SAMPLE_FMT_S16, 0);
                    out_samples = swr_convert(swrCtx, &resampled_data, out_samples, (const uint8_t**)frame->data, frame->nb_samples);
                    int data_size = out_samples * 2 * 2;
                    QByteArray audioData((char*)resampled_data, data_size);
                    av_freep(&resampled_data);
                    QMutexLocker locker(&mutex);
                    audioQueue.enqueue(audioData);
                }
            }
        }
        av_packet_unref(packet);
    }
    av_packet_free(&packet); av_frame_free(&frame); sws_freeContext(swsCtx); swr_free(&swrCtx);
    avcodec_free_context(&videoCodecCtx); avcodec_free_context(&audioCodecCtx); avformat_close_input(&formatCtx);
    qDebug() << "解码线程已结束。";
}

VideoProcessor::VideoProcessor(Ui::MainWindow *ui, QObject *parent)
    : QObject(parent), ui(ui)
{
    videoListModel = new QStringListModel(this);
    ui->videoListView->setModel(videoListModel);
    ui->speedComboBox->addItems({"0.5x", "1.0x", "1.5x", "2.0x"});
    ui->speedComboBox->setCurrentIndex(1);
    ui->filterComboBox->addItems({"无", "模糊", "锐化"});
    loadFaceDetector(); // [关键变更] 调用新的加载方法
    ui->controlBar->setEnabled(false);
    ui->videoEffectsToolBox->setEnabled(false);
}

VideoProcessor::~VideoProcessor() { stopCurrentVideo(); }

void VideoProcessor::stopCurrentVideo() {
    if (displayTimer) {
        displayTimer->stop();
        delete displayTimer;
        displayTimer = nullptr;
    }
    if (decoderThread) {
        decoderThread->stop();
        decoderThread->wait(1000);
        delete decoderThread;
        decoderThread = nullptr;
    }
    if (audioSink) {
        disconnect(audioSink, &QAudioSink::stateChanged, this, &VideoProcessor::handleAudioStateChange);
        audioSink->stop();
        delete audioSink;
        audioSink = nullptr;
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
    stopCurrentVideo();
    QString filePath = videoListModel->data(index, Qt::DisplayRole).toString();
    decoderThread = new VideoDecoder(this);
    connect(decoderThread, &VideoDecoder::seekFinished, this, &VideoProcessor::onSeekFinished);
    if (!decoderThread->startDecoding(filePath)) {
        QMessageBox::critical(qobject_cast<QWidget*>(parent()), "错误", "无法打开或解析视频文件。");
        stopCurrentVideo();
        return;
    }
    videoDurationMs = decoderThread->getDurationMs();
    double fps = decoderThread->getFPS();
    audioFormat.setSampleRate(48000);
    audioFormat.setChannelCount(2);
    audioFormat.setSampleFormat(QAudioFormat::Int16);
    recreateAudioSink();
    displayTimer = new QTimer(this);
    connect(displayTimer, &QTimer::timeout, this, &VideoProcessor::updateDisplay);
    displayTimer->start(1000.0 / fps);
    isVideoPlaying = true;
    emit videoOpened(true, videoDurationMs, fps);
    ui->controlBar->setEnabled(true);
    ui->videoEffectsToolBox->setEnabled(true);
    updatePlayPauseButton(true);
}

void VideoProcessor::togglePlayPause() {
    if (!decoderThread || isSeeking) return;
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
    while(audioDevice && audioSink && audioSink->state() != QAudio::StoppedState && audioSink->bytesFree() > 0) {
        QByteArray audioData = decoderThread->getAudioChunk();
        if (audioData.isEmpty()) break;
        audioDevice->write(audioData);
    }
    qint64 audioPts = audioSink ? (audioSink->processedUSecs() / 1000) : 0;
    cv::Mat frame = decoderThread->getVideoFrame(audioPts);
    if (frame.empty()) return;
    cv::Mat processedFrame = applyEffects(frame);
    currentPixmap = QPixmap::fromImage(ImageConverter::matToQImage(processedFrame));
    emit frameReady(currentPixmap);
    emit progressUpdated(QString("%1 / %2").arg(formatTime(audioPts)).arg(formatTime(videoDurationMs)), audioPts, videoDurationMs);
}

void VideoProcessor::onSliderPressed() {
    if (!decoderThread) return;
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
    isSeeking = true;
    qDebug() << "Seek requested to:" << ui->videoSlider->value() << "ms. Notifying decoder thread.";
    decoderThread->seek(ui->videoSlider->value());
}

void VideoProcessor::onSeekFinished() {
    qDebug() << "Decoder has finished seek. Now stopping audio sink...";
    if (audioSink) {
        audioSink->stop();
    }
}

void VideoProcessor::handleAudioStateChange(QAudio::State state) {
    qDebug() << "Audio state changed to:" << state;
    if (isSeeking && state == QAudio::StoppedState) {
        qDebug() << "Audio sink confirmed stopped. Forcing recreation.";
        recreateAudioSink();
    }
}

void VideoProcessor::recreateAudioSink() {
    qDebug() << "Recreating AudioSink instance...";
    if (audioSink) {
        disconnect(audioSink, &QAudioSink::stateChanged, this, &VideoProcessor::handleAudioStateChange);
        delete audioSink;
        audioSink = nullptr;
    }
    const QAudioDevice &defaultAudioDevice = QMediaDevices::defaultAudioOutput();
    if (defaultAudioDevice.isNull()) {
        qCritical() << "Cannot recreate audio sink, no default device found.";
        isSeeking = false;
        return;
    }
    audioSink = new QAudioSink(defaultAudioDevice, audioFormat, this);
    connect(audioSink, &QAudioSink::stateChanged, this, &VideoProcessor::handleAudioStateChange);
    audioDevice = audioSink->start();
    if (!audioDevice) {
        qCritical() << "Recreated audio sink FAILED to start. Error:" << audioSink->error();
        isSeeking = false;
        return;
    }
    qDebug() << "Recreated audio sink started successfully.";
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
