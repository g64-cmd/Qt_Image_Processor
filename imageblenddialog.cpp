#include "imageblenddialog.h"
#include "ui_imageblenddialog.h"
#include "imageprocessor.h"
#include <QFileDialog>
#include <QPushButton> // <-- 新增：用于获取具体的按钮

ImageBlendDialog::ImageBlendDialog(const QPixmap &initialPixmap, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageBlendDialog),
    pixmapA(initialPixmap)
{
    ui->setupUi(this);
    setWindowTitle("图像融合");

    // 1. 显示初始图片 A
    ui->labelImageA->setPixmap(pixmapA.scaled(ui->labelImageA->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // 2. 设置滑块范围 (0-100) 和初始值 (50)
    ui->sliderBlend->setRange(0, 100);
    ui->sliderBlend->setValue(50);

    // --- 关键修复：手动连接 Apply 按钮 ---
    // QDialogButtonBox 的 "Apply" 按钮默认不会触发 accept()。我们需要手动连接。
    QPushButton *applyButton = ui->buttonBox->button(QDialogButtonBox::Apply);
    if (applyButton) {
        // 将点击信号连接到 accept() 槽
        connect(applyButton, &QPushButton::clicked, this, &ImageBlendDialog::accept);
    }

    // 3. 初始状态：由于没有图片B，最终效果就是图片A
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
        // 显示图片B的缩略图
        ui->labelImageB->setPixmap(pixmapB.scaled(ui->labelImageB->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        // 立即更新融合效果
        updateBlendedImage();
    }
}

void ImageBlendDialog::on_sliderBlend_valueChanged(int value)
{
    // 只要滑块一动，就立即更新融合效果
    updateBlendedImage();
}

void ImageBlendDialog::updateBlendedImage()
{
    if (pixmapA.isNull()) return;

    // 如果图片B还未加载，则最终效果就是图片A
    if (pixmapB.isNull()) {
        blendedPixmap = pixmapA;
    } else {
        // 计算权重 (0.0 - 1.0)
        double alpha = ui->sliderBlend->value() / 100.0;
        // 调用我们强大的图像处理器
        QImage resultImage = ImageProcessor::blend(pixmapA.toImage(), pixmapB.toImage(), alpha);
        if (!resultImage.isNull()) {
            blendedPixmap = QPixmap::fromImage(resultImage);
        }
    }

    // 将最终结果显示在中央预览区
    if (!blendedPixmap.isNull()) {
        ui->labelResult->setPixmap(blendedPixmap.scaled(ui->labelResult->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}
