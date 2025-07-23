// beautydialog.h
#ifndef BEAUTYDIALOG_H
#define BEAUTYDIALOG_H

#include <QDialog>
#include <QPixmap>

namespace Ui {
class BeautyDialog;
}
class BeautyProcessor;

class BeautyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BeautyDialog(const QPixmap &initialPixmap, QWidget *parent = nullptr);
    ~BeautyDialog();

    QPixmap getResultImage() const;

private slots:
    void on_sliderSmooth_valueChanged(int value);
    void on_sliderThin_valueChanged(int value);

private:
    void applyBeautyFilter();

    Ui::BeautyDialog *ui;
    BeautyProcessor *processor;
    QPixmap originalPixmap;
    QPixmap resultPixmap;
};

#endif // BEAUTYDIALOG_H
