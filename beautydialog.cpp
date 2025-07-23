// beautydialog.cpp
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

    ui->labelBefore->setPixmap(originalPixmap.scaled(ui->labelBefore->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    ui->sliderSmooth->setRange(0, 100);
    ui->sliderSmooth->setValue(50);
    ui->sliderThin->setRange(0, 100);
    ui->sliderThin->setValue(0);

    QPushButton *applyButton = ui->buttonBox->button(QDialogButtonBox::Apply);
    if (applyButton) {
        connect(applyButton, &QPushButton::clicked, this, &BeautyDialog::accept);
    }

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
    (void)value;
    applyBeautyFilter();
}

void BeautyDialog::on_sliderThin_valueChanged(int value)
{
    (void)value;
    applyBeautyFilter();
}

void BeautyDialog::applyBeautyFilter()
{
    int smoothLevel = ui->sliderSmooth->value();
    int thinLevel = ui->sliderThin->value();

    QImage result = processor->process(originalPixmap.toImage(), smoothLevel, thinLevel);
    if (!result.isNull()) {
        resultPixmap = QPixmap::fromImage(result);
        ui->labelAfter->setPixmap(resultPixmap.scaled(ui->labelAfter->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}
