#include "imagetexturetransferdialog.h"
#include "ui_imagetexturetransferdialog.h" // 這個文件名是根據 .ui 文件名生成的，所以是小寫
#include "imageprocessor.h"
#include <QFileDialog>
#include <QPushButton>

ImageTextureTransferDialog::ImageTextureTransferDialog(const QPixmap &contentPixmap, QWidget *parent) :
    QDialog(parent),
    // --- 關鍵修復：使用正確的小寫 UI 類 ---
    ui(new Ui::imagetexturetransferdialog),
    contentPixmap(contentPixmap)
{
    ui->setupUi(this);
    setWindowTitle("紋理遷移");

    // 顯示內容圖
    ui->labelContent->setPixmap(contentPixmap.scaled(ui->labelContent->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // 手動連接 Apply 按鈕
    QPushButton *applyButton = ui->buttonBox->button(QDialogButtonBox::Apply);
    if (applyButton) {
        connect(applyButton, &QPushButton::clicked, this, &ImageTextureTransferDialog::accept);
    }

    // 預設隱藏進度條
    ui->progressBar->setVisible(false);
}

ImageTextureTransferDialog::~ImageTextureTransferDialog()
{
    delete ui;
}

QPixmap ImageTextureTransferDialog::getResultImage() const
{
    return resultPixmap;
}

void ImageTextureTransferDialog::on_buttonOpenTexture_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "選擇紋理圖", "", "Image Files (*.png *.jpg *.bmp)");
    if (!fileName.isEmpty()) {
        texturePixmap.load(fileName);
        ui->labelTexture->setPixmap(texturePixmap.scaled(ui->labelTexture->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        // 選擇紋理圖後立即應用效果
        applyTextureTransfer();
    }
}

void ImageTextureTransferDialog::applyTextureTransfer()
{
    if (contentPixmap.isNull() || texturePixmap.isNull()) return;

    // 控制進度條
    ui->labelResult->setText("正在處理中，請稍候...");
    ui->progressBar->setVisible(true);
    ui->progressBar->setRange(0, 0);
    QApplication::processEvents();

    QImage result = ImageProcessor::textureTransfer(contentPixmap.toImage(), texturePixmap.toImage());

    ui->progressBar->setVisible(false);

    if (!result.isNull()) {
        resultPixmap = QPixmap::fromImage(result);
        ui->labelResult->setPixmap(resultPixmap.scaled(ui->labelResult->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        ui->labelResult->setText("處理失敗");
    }
}
