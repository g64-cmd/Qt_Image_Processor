// newstitcherdialog.cpp
#include "newstitcherdialog.h"
#include "ui_newstitcherdialog.h"
#include "imageconverter.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QListWidgetItem>
#include <vector>
#include <QPushButton>
#include <QTimer>
#include <QDebug> // 引入Debug头文件

NewStitcherDialog::NewStitcherDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewStitcherDialog)
{
    ui->setupUi(this);
    setWindowTitle("基于特征点的图像拼接");

    // --- FINAL FIX: Disconnect the button box's default 'accepted' signal ---
    // This is the root cause of the dialog closing prematurely.
    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    QPushButton* stitchButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    stitchButton->setText("开始拼接");
    // Now, only our custom slot will be called when this button is clicked.
    connect(stitchButton, &QPushButton::clicked, this, &NewStitcherDialog::on_stitchButtonClicked);

    ui->progressBar->setVisible(false);
    updateButtonStates();

    connect(ui->imageListWidget, &QListWidget::itemSelectionChanged, this, &NewStitcherDialog::updateButtonStates);
    qDebug() << "NewStitcherDialog constructed.";
}

NewStitcherDialog::~NewStitcherDialog()
{
    qDebug() << "NewStitcherDialog destructed.";
    delete ui;
}

QPixmap NewStitcherDialog::getResultImage() const
{
    return resultPixmap;
}

void NewStitcherDialog::on_addButton_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "选择要拼接的图片 (按顺序)",
        "",
        "Image Files (*.png *.jpg *.jpeg *.bmp)");

    if (!files.isEmpty()) {
        ui->imageListWidget->addItems(files);
        imagePaths.append(files);
        updateButtonStates();
    }
}

void NewStitcherDialog::on_removeButton_clicked()
{
    QList<QListWidgetItem*> selectedItems = ui->imageListWidget->selectedItems();
    if (selectedItems.isEmpty()) return;

    for (QListWidgetItem* item : selectedItems) {
        imagePaths.removeOne(item->text());
        delete ui->imageListWidget->takeItem(ui->imageListWidget->row(item));
    }
    updateButtonStates();
}

void NewStitcherDialog::on_moveUpButton_clicked()
{
    int currentRow = ui->imageListWidget->currentRow();
    if (currentRow > 0) {
        QListWidgetItem *item = ui->imageListWidget->takeItem(currentRow);
        ui->imageListWidget->insertItem(currentRow - 1, item);
        imagePaths.move(currentRow, currentRow - 1);
        ui->imageListWidget->setCurrentRow(currentRow - 1);
    }
}

void NewStitcherDialog::on_moveDownButton_clicked()
{
    int currentRow = ui->imageListWidget->currentRow();
    if (currentRow < ui->imageListWidget->count() - 1) {
        QListWidgetItem *item = ui->imageListWidget->takeItem(currentRow);
        ui->imageListWidget->insertItem(currentRow + 1, item);
        imagePaths.move(currentRow, currentRow + 1);
        ui->imageListWidget->setCurrentRow(currentRow + 1);
    }
}

void NewStitcherDialog::on_stitchButtonClicked()
{
    if (imagePaths.count() < 2) {
        QMessageBox::warning(this, "数量不足", "请至少选择两张图片进行拼接。");
        return;
    }

    std::vector<cv::Mat> images;
    for (const QString &path : imagePaths) {
        cv::Mat img = cv::imread(path.toLocal8Bit().constData());
        if (img.empty()) {
            QMessageBox::critical(this, "加载失败", QString("无法加载图片: %1").arg(path));
            return;
        }
        images.push_back(img);
    }

    // --- 禁用UI ---
    ui->progressBar->setVisible(true);
    ui->buttonBox->setEnabled(false);
    ui->addButton->setEnabled(false);
    ui->removeButton->setEnabled(false);
    ui->moveUpButton->setEnabled(false);
    ui->moveDownButton->setEnabled(false);
    ui->imageListWidget->setEnabled(false);

    StitcherThread* worker = new StitcherThread(images, this);

    // --- 线程生命周期管理 ---
    connect(worker, &StitcherThread::resultReady, this, [this](const cv::Mat& resultMat) {
        qDebug() << "Thread finished processing, resultReady signal received.";
        ui->progressBar->setVisible(false);

        if (resultMat.empty()) {
            QMessageBox::critical(this, "拼接失败", "无法拼接所选图片，请确保图片之间有足够的重叠区域且顺序正确。");
        } else {
            resultPixmap = QPixmap::fromImage(ImageConverter::matToQImage(resultMat));
            ui->previewLabel->setPixmap(resultPixmap.scaled(ui->previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    });

    connect(worker, &QThread::finished, this, [this](){
        qDebug() << "Thread finished signal received. UI is now safe to modify.";
        // --- 恢复UI ---
        ui->buttonBox->setEnabled(true);
        ui->imageListWidget->setEnabled(true);
        updateButtonStates();

        // 如果拼接成功 (resultPixmap不为空), 更改按钮功能
        if (!resultPixmap.isNull()) {
            QPushButton* okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
            okButton->setText("确定");
            disconnect(okButton, &QPushButton::clicked, this, &NewStitcherDialog::on_stitchButtonClicked);
            connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
        }
    });

    connect(worker, &QThread::finished, worker, &QObject::deleteLater);

    qDebug() << "Starting stitcher thread...";
    worker->start();
}

void NewStitcherDialog::updateButtonStates()
{
    bool hasSelection = !ui->imageListWidget->selectedItems().isEmpty();
    ui->removeButton->setEnabled(hasSelection);
    ui->moveUpButton->setEnabled(hasSelection);
    ui->moveDownButton->setEnabled(hasSelection);
    ui->addButton->setEnabled(true);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->imageListWidget->count() >= 2);
}
