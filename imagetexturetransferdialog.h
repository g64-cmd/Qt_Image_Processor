#ifndef IMAGETEXTURETRANSFERDIALOG_H
#define IMAGETEXTURETRANSFERDIALOG_H

#include <QDialog>
#include <QPixmap>

// --- 關鍵修復：將命名空間改為小寫以匹配 .ui 文件 ---
namespace Ui {
class imagetexturetransferdialog;
}

class ImageTextureTransferDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImageTextureTransferDialog(const QPixmap &contentPixmap, QWidget *parent = nullptr);
    ~ImageTextureTransferDialog();

    QPixmap getResultImage() const;

private slots:
    void on_buttonOpenTexture_clicked();

private:
    void applyTextureTransfer();

    // --- 關鍵修復：使用正確的小寫 UI 類 ---
    Ui::imagetexturetransferdialog *ui;
    QPixmap contentPixmap;
    QPixmap texturePixmap;
    QPixmap resultPixmap;
};

#endif // IMAGETEXTURETRANSFERDIALOG_H
