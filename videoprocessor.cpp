// videoprocessor.cpp
#include "videoprocessor.h"
#include "ui_mainwindow.h"
#include "imageconverter.h"
#include "imageprocessor.h"
#include <QTimer>
#include <QStringListModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QTemporaryFile>

VideoProcessor::VideoProcessor(Ui::MainWindow *ui, QObject *parent)
    : QObject(parent), ui(ui), isVideoPlaying(false), isSliderBeingDragged(false), currentPlaybackSpeed(1.0), isRecording(false)
{
    // 1. Initialize members
    videoTimer = new QTimer(this);
    videoListModel = new QStringListModel(this);
    ui->videoListView->setModel(videoListModel);

    // 2. Configure UI elements
    ui->speedComboBox->addItems({"0.5x", "1.0x", "1.5x", "2.0x"});
    ui->speedComboBox->setCurrentIndex(1); // Default to 1.0x
    ui->filterComboBox->addItems({"无", "模糊", "锐化"});

    // 3. Connect internal signals and slots
    connect(videoTimer, &QTimer::timeout, this, &VideoProcessor::processNextFrame);

    // 4. Load necessary models
    loadFaceCascade();

    // 5. Set initial UI state
    ui->controlBar->setEnabled(false);
    ui->videoEffectsToolBox->setEnabled(false);
}

VideoProcessor::~VideoProcessor()
{
    if (videoCapture.isOpened()) {
        videoCapture.release();
    }
}

void VideoProcessor::loadFaceCascade()
{
    QFile cascadeFile(":/resources/haarcascades/haarcascade_frontalface_alt.xml");
    if (cascadeFile.open(QIODevice::ReadOnly)) {
        QTemporaryFile tempFile;
        if (tempFile.open()) {
            tempFile.write(cascadeFile.readAll());
            tempFile.close();
            if (!faceCascade.load(tempFile.fileName().toStdString())) {
                qWarning() << "Could not load face cascade for video.";
            } else {
                qDebug() << "Successfully loaded face cascade for video.";
            }
        }
        cascadeFile.close();
    } else {
        qWarning() << "Could not find face cascade resource for video.";
    }
}

void VideoProcessor::addVideos()
{
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

void VideoProcessor::removeSelectedVideo()
{
    QModelIndexList selected = ui->videoListView->selectionModel()->selectedIndexes();
    std::sort(selected.begin(), selected.end(), std::greater<QModelIndex>());
    for (const QModelIndex& index : selected) {
        videoListModel->removeRow(index.row());
    }
}

void VideoProcessor::playVideoAtIndex(const QModelIndex &index)
{
    QString filePath = videoListModel->data(index, Qt::DisplayRole).toString();
    if (videoCapture.isOpened()) {
        videoCapture.release();
    }

    if (!videoCapture.open(filePath.toLocal8Bit().constData())) {
        QMessageBox::critical(qobject_cast<QWidget*>(parent()), "错误", "无法打开视频文件。");
        return;
    }

    int frameCount = static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_COUNT));
    double fps = videoCapture.get(cv::CAP_PROP_FPS);

    emit videoOpened(true, frameCount - 1, fps);

    ui->controlBar->setEnabled(true);
    ui->videoEffectsToolBox->setEnabled(true);

    isVideoPlaying = true;
    videoTimer->start(1000.0 / (fps * currentPlaybackSpeed));
    updatePlayPauseButton(true);
}

void VideoProcessor::togglePlayPause()
{
    if (!videoCapture.isOpened()) return;

    isVideoPlaying = !isVideoPlaying;
    if (isVideoPlaying) {
        videoTimer->start();
    } else {
        videoTimer->stop();
    }
    updatePlayPauseButton(isVideoPlaying);
}

void VideoProcessor::processNextFrame()
{
    if (isSliderBeingDragged) return; // Don't process while user is seeking

    if (!videoCapture.read(currentFrame) || currentFrame.empty()) {
        videoTimer->stop();
        isVideoPlaying = false;
        videoCapture.set(cv::CAP_PROP_POS_FRAMES, 0); // Rewind
        updatePlayPauseButton(false);
        return;
    }

    cv::Mat processedFrame = applyEffects(currentFrame);
    currentPixmap = QPixmap::fromImage(ImageConverter::matToQImage(processedFrame));

    emit frameReady(currentPixmap);

    int currentFramePos = static_cast<int>(videoCapture.get(cv::CAP_PROP_POS_FRAMES));
    int totalFrames = static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_COUNT));
    double fps = videoCapture.get(cv::CAP_PROP_FPS);

    if (fps > 0) {
        int currentTimeSec = currentFramePos / fps;
        int totalTimeSec = totalFrames / fps;
        QString timeString = QString("%1:%2 / %3:%4")
                                 .arg(currentTimeSec / 60, 2, 10, QChar('0'))
                                 .arg(currentTimeSec % 60, 2, 10, QChar('0'))
                                 .arg(totalTimeSec / 60, 2, 10, QChar('0'))
                                 .arg(totalTimeSec % 60, 2, 10, QChar('0'));
        emit progressUpdated(timeString, currentFramePos, totalFrames);
    }
}

cv::Mat VideoProcessor::applyEffects(const cv::Mat &frame)
{
    cv::Mat result = frame.clone();

    int b = ui->videoBrightnessSlider->value();
    int c = ui->videoContrastSlider->value();
    int s = ui->videoSaturationSlider->value();
    int h = ui->videoHueSlider->value();
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

    if (ui->faceDetectCheckBox->isChecked() && !faceCascade.empty()) {
        cv::Mat gray;
        cv::cvtColor(result, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, gray);
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(30, 30));
        for (const auto& face : faces) {
            cv::rectangle(result, face, cv::Scalar(0, 255, 0), 2);
        }
    }

    // ... Other filters can be added here ...

    return result;
}

void VideoProcessor::updatePlayPauseButton(bool isPlaying)
{
    if (isPlaying) {
        ui->playPauseButton->setText("暂停");
        ui->playPauseButton->setIcon(QIcon(":/icons/resources/icons/pause.svg"));
    } else {
        ui->playPauseButton->setText("播放");
        ui->playPauseButton->setIcon(QIcon(":/icons/resources/icons/play.svg"));
    }
}

void VideoProcessor::seek(int position)
{
    if (videoCapture.isOpened()) {
        isSliderBeingDragged = true;
        videoTimer->stop();
        videoCapture.set(cv::CAP_PROP_POS_FRAMES, position);
        // Process one frame immediately to update the view
        processNextFrame();
    }
}

void VideoProcessor::stopSeeking()
{
    isSliderBeingDragged = false;
    if (videoCapture.isOpened() && isVideoPlaying) {
        videoTimer->start();
    }
}

void VideoProcessor::setSpeed(int index)
{
    switch (index) {
    case 0: currentPlaybackSpeed = 0.5; break;
    case 1: currentPlaybackSpeed = 1.0; break;
    case 2: currentPlaybackSpeed = 1.5; break;
    case 3: currentPlaybackSpeed = 2.0; break;
    default: currentPlaybackSpeed = 1.0; break;
    }

    if (videoCapture.isOpened()) {
        double fps = videoCapture.get(cv::CAP_PROP_FPS);
        videoTimer->setInterval(1000.0 / (fps * currentPlaybackSpeed));
    }
}

void VideoProcessor::saveCurrentFrame()
{
    if (currentPixmap.isNull()) {
        QMessageBox::warning(qobject_cast<QWidget*>(parent()), "无内容", "没有可保存的视频帧。");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(qobject_cast<QWidget*>(parent()), "保存当前帧", "", "PNG Image (*.png)");
    if (!fileName.isEmpty()) {
        currentPixmap.save(fileName);
    }
}

void VideoProcessor::toggleRecording()
{
    // Implementation for recording will be added in the next step
    QMessageBox::information(qobject_cast<QWidget*>(parent()), "待实现", "录制功能将在下一步中实现。");
    ui->recordButton->setChecked(false); // Reset button state for now
}
