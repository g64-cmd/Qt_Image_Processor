#ifndef IMAGEBLENDDIALOG_H
#define IMAGEBLENDDIALOG_H

#include <QDialog>
#include <QPixmap>

namespace Ui {
class ImageBlendDialog;
}

class ImageBlendDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImageBlendDialog(const QPixmap &initialPixmap, QWidget *parent = nullptr);
    ~ImageBlendDialog();
    QPixmap getBlendedImage() const;

private slots:
    void on_buttonOpenImageB_clicked();
    void on_sliderBlend_valueChanged(int value);

private:
    void updateBlendedImage();
    Ui::ImageBlendDialog *ui;
    QPixmap pixmapA;
    QPixmap pixmapB;
    QPixmap blendedPixmap;
};

#endif // IMAGEBLENDDIALOG_H
