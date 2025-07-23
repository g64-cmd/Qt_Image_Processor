// newstitcherdialog.h
#ifndef NEWSTITCHERDIALOG_H
#define NEWSTITCHERDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QPixmap>
#include <QThread>
#include <opencv2/opencv.hpp>
#include "imagestitcherprocessor.h"

// --- FIX: Moved StitcherThread class definition to the header file ---
// This allows Qt's Meta-Object Compiler (moc) to process it correctly.
class StitcherThread : public QThread
{
    Q_OBJECT
public:
    StitcherThread(const std::vector<cv::Mat>& imgs, QObject* parent = nullptr)
        : QThread(parent), images(imgs) {}

    void run() override {
        ImageStitcherProcessor processor;
        cv::Mat result = processor.process(images);
        emit resultReady(result);
    }

signals:
    void resultReady(const cv::Mat& image);

private:
    std::vector<cv::Mat> images;
};


// --- 向前声明 ---
namespace Ui {
class NewStitcherDialog;
}

class NewStitcherDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewStitcherDialog(QWidget *parent = nullptr);
    ~NewStitcherDialog();

    QPixmap getResultImage() const;

private slots:
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_moveUpButton_clicked();
    void on_moveDownButton_clicked();
    void on_stitchButtonClicked(); // 连接到“拼接”按钮

private:
    void updateButtonStates();

    Ui::NewStitcherDialog *ui;
    QStringList imagePaths;
    QPixmap resultPixmap;
};

#endif // NEWSTITCHERDIALOG_H
