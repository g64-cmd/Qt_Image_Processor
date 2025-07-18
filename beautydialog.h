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
    // 只保留磨皮滑块的槽函数
    void on_sliderSmooth_valueChanged(int value);

private:
    void applyBeautyFilter();

    Ui::BeautyDialog *ui;
    BeautyProcessor *processor;
    QPixmap originalPixmap;
    QPixmap resultPixmap;
};

#endif // BEAUTYDIALOG_H
