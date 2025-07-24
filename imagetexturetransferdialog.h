#ifndef IMAGETEXTURETRANSFERDIALOG_H
#define IMAGETEXTURETRANSFERDIALOG_H

#include <QDialog>
#include <QPixmap>

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
    Ui::imagetexturetransferdialog *ui;
    QPixmap contentPixmap;
    QPixmap texturePixmap;
    QPixmap resultPixmap;
};

#endif // IMAGETEXTURETRANSFERDIALOG_H
