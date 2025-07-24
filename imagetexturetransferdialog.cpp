#include "imagetexturetransferdialog.h"
#include "ui_imagetexturetransferdialog.h"
#include "imageprocessor.h"
#include <QFileDialog>
#include <QPushButton>

ImageTextureTransferDialog::ImageTextureTransferDialog(const QPixmap &contentPixmap, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::imagetexturetransferdialog),
    contentPixmap(contentPixmap)
{
    ui->setupUi(this);
    setWindowTitle("纹理迁移");
    ui->labelContent->setPixmap(contentPixmap.scaled(ui->labelContent->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QPushButton *applyButton = ui->buttonBox->button(QDialogButtonBox::Apply);
    if (applyButton) {
        connect(applyButton, &QPushButton::clicked, this, &ImageTextureTransferDialog::accept);
    }
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
    QString fileName = QFileDialog::getOpenFileName(this, "选择纹理图", "", "Image Files (*.png *.jpg *.bmp)");
    if (!fileName.isEmpty()) {
        texturePixmap.load(fileName);
        ui->labelTexture->setPixmap(texturePixmap.scaled(ui->labelTexture->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        applyTextureTransfer();
    }
}

void ImageTextureTransferDialog::applyTextureTransfer()
{
    if (contentPixmap.isNull() || texturePixmap.isNull()) return;
    ui->labelResult->setText("正在处理中，请稍后！");
    ui->progressBar->setVisible(true);
    ui->progressBar->setRange(0, 0);
    QApplication::processEvents();
    QImage result = ImageProcessor::textureTransfer(contentPixmap.toImage(), texturePixmap.toImage());
    ui->progressBar->setVisible(false);
    if (!result.isNull()) {
        resultPixmap = QPixmap::fromImage(result);
        ui->labelResult->setPixmap(resultPixmap.scaled(ui->labelResult->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        ui->labelResult->setText("处理失败");
    }
}
