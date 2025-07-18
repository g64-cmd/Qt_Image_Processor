#include "beautydialog.h"
#include "ui_beautydialog.h"
#include "beautyprocessor.h"
#include <QPushButton>

BeautyDialog::BeautyDialog(const QPixmap &initialPixmap, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BeautyDialog),
    originalPixmap(initialPixmap)
{
    ui->setupUi(this);
    setWindowTitle("美颜工作室");

    processor = new BeautyProcessor();

    // 显示原图
    ui->labelBefore->setPixmap(originalPixmap.scaled(ui->labelBefore->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // 设置磨皮滑块
    ui->sliderSmooth->setRange(0, 100);
    ui->sliderSmooth->setValue(50); // 默认中等强度

    // 手动连接Apply按钮
    QPushButton *applyButton = ui->buttonBox->button(QDialogButtonBox::Apply);
    if (applyButton) {
        connect(applyButton, &QPushButton::clicked, this, &BeautyDialog::accept);
    }

    // 立即应用一次默认效果
    applyBeautyFilter();
}

BeautyDialog::~BeautyDialog()
{
    delete ui;
    delete processor;
}

QPixmap BeautyDialog::getResultImage() const
{
    return resultPixmap;
}

void BeautyDialog::on_sliderSmooth_valueChanged(int value)
{
    // 避免未使用的参数警告
    (void)value;
    applyBeautyFilter();
}

void BeautyDialog::applyBeautyFilter()
{
    int smoothLevel = ui->sliderSmooth->value();

    // 调用简化的 process 函数，只传递磨皮等级
    QImage result = processor->process(originalPixmap.toImage(), smoothLevel);
    if (!result.isNull()) {
        resultPixmap = QPixmap::fromImage(result);
        ui->labelAfter->setPixmap(resultPixmap.scaled(ui->labelAfter->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}
