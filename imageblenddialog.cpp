#include "imageblenddialog.h"
#include "ui_imageblenddialog.h"
#include "imageprocessor.h"
#include <QFileDialog>
#include <QPushButton>

ImageBlendDialog::ImageBlendDialog(const QPixmap &initialPixmap, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageBlendDialog),
    pixmapA(initialPixmap)
{
    ui->setupUi(this);
    setWindowTitle("图像融合");
    ui->labelImageA->setPixmap(pixmapA.scaled(ui->labelImageA->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->sliderBlend->setRange(0, 100);
    ui->sliderBlend->setValue(50);
    QPushButton *applyButton = ui->buttonBox->button(QDialogButtonBox::Apply);
    if (applyButton) {
        connect(applyButton, &QPushButton::clicked, this, &ImageBlendDialog::accept);
    }
    updateBlendedImage();
}

ImageBlendDialog::~ImageBlendDialog()
{
    delete ui;
}

QPixmap ImageBlendDialog::getBlendedImage() const
{
    return blendedPixmap;
}

void ImageBlendDialog::on_buttonOpenImageB_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择图片 B", "", "Image Files (*.png *.jpg *.bmp)");
    if (!fileName.isEmpty()) {
        pixmapB.load(fileName);
        ui->labelImageB->setPixmap(pixmapB.scaled(ui->labelImageB->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        updateBlendedImage();
    }
}

void ImageBlendDialog::on_sliderBlend_valueChanged(int value)
{
    updateBlendedImage();
}

void ImageBlendDialog::updateBlendedImage()
{
    if (pixmapA.isNull()) return;
    if (pixmapB.isNull()) {
        blendedPixmap = pixmapA;
    } else {
        double alpha = ui->sliderBlend->value() / 100.0;
        QImage resultImage = ImageProcessor::blend(pixmapA.toImage(), pixmapB.toImage(), alpha);
        if (!resultImage.isNull()) {
            blendedPixmap = QPixmap::fromImage(resultImage);
        }
    }
    if (!blendedPixmap.isNull()) {
        ui->labelResult->setPixmap(blendedPixmap.scaled(ui->labelResult->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}
