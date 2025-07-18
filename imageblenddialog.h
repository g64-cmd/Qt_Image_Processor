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

    // 公共接口，用于让主窗口获取最终的融合结果
    QPixmap getBlendedImage() const;

private slots:
    // 响应“打开图片B”按钮的点击
    void on_buttonOpenImageB_clicked();
    // 响应滑块数值的变化
    void on_sliderBlend_valueChanged(int value);

private:
    // 核心的私有函数，用于更新预览
    void updateBlendedImage();

    Ui::ImageBlendDialog *ui;
    QPixmap pixmapA;
    QPixmap pixmapB;
    QPixmap blendedPixmap; // 存储当前的融合结果
};

#endif // IMAGEBLENDDIALOG_H
