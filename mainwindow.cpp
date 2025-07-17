#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "imageprocessor.h" // <<< 1. 包含图像处理器头文件

#include <QFileDialog>
#include <QMessageBox>
#include <QImageReader>
#include <QWheelEvent>
#include <QGuiApplication>
#include <QFileInfo>
#include <QtMath>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scaleFactor(1.0)
    , imageScene(nullptr)
    , pixmapItem(nullptr)
{
    ui->setupUi(this);

    imageScene = new QGraphicsScene(this);
    ui->graphicsView->setScene(imageScene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);
    ui->graphicsView->setAlignment(Qt::AlignCenter);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    ui->graphicsView->viewport()->installEventFilter(this);

    // <<< 2. 连接信号和槽 (如果使用自动连接，这一步是可选的，但写出来更清晰)
    connect(ui->imageSharpenButton, &QPushButton::clicked, this, &MainWindow::on_imageSharpenButton_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionopen_triggered()
{
    // ... (这部分代码保持不变)
    QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();
    QString filter = "Image Files (";
    for (const QByteArray &format : supportedFormats) {
        filter += "*." + QString(format) + " ";
    }
    filter += ");;All Files (*)";

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("打开图像"), QDir::homePath(), filter);

    if (fileName.isEmpty()) {
        return;
    }

    currentFilePath = fileName;

    if (!originalPixmap.load(currentFilePath)) {
        QMessageBox::critical(this, tr("错误"), tr("无法加载图像文件: %1").arg(currentFilePath));
        originalPixmap = QPixmap();
        currentFilePath.clear();
        return;
    }

    // <<< 3. 打开新图片时，将处理后的图片也重置为原始图片
    processedPixmap = originalPixmap;

    updateDisplayImage(processedPixmap); // 使用新函数更新显示
    fitToWindow();
    updateImageInfo();
    ui->statusbar->showMessage(tr("成功打开: %1").arg(currentFilePath), 5000);
}

// <<< 4. 新增的槽函数实现
void MainWindow::on_imageSharpenButton_clicked()
{
    if (processedPixmap.isNull()) {
        QMessageBox::information(this, "提示", "请先打开一张图片。");
        return;
    }

    // 将当前处理后的图像转换为 QImage 进行锐化
    QImage sourceImage = processedPixmap.toImage();
    QImage sharpenedImage = ImageProcessor::sharpen(sourceImage);

    if (sharpenedImage.isNull()) {
        QMessageBox::warning(this, "错误", "图像锐化失败。");
        return;
    }

    // 更新处理后的图像
    processedPixmap = QPixmap::fromImage(sharpenedImage);

    // 更新显示
    updateDisplayImage(processedPixmap);
    ui->statusbar->showMessage("图像锐化完成", 3000);
}

// <<< 5. 新增的图像更新函数
/**
 * @brief 使用给定的 pixmap 更新场景和显示
 * @param pixmap 要显示的 QPixmap
 */
void MainWindow::updateDisplayImage(const QPixmap &pixmap)
{
    if (pixmap.isNull()) return;

    // 清空场景并添加新的 pixmap
    imageScene->clear();
    pixmapItem = imageScene->addPixmap(pixmap);
    imageScene->setSceneRect(pixmap.rect());
}


// --- 以下函数保持不变 ---

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->graphicsView->viewport() && event->type() == QEvent::Wheel && pixmapItem) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        int angle = wheelEvent->angleDelta().y();
        double zoomFactor = (angle > 0) ? 1.15 : (1.0 / 1.15);
        scaleImage(scaleFactor * zoomFactor);
        return true;
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (!pixmapItem) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    if (event->modifiers() == Qt::ControlModifier) {
        if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
            scaleImage(scaleFactor * 1.2);
        } else if (event->key() == Qt::Key_Minus) {
            scaleImage(scaleFactor * 0.8);
        } else {
            QMainWindow::keyPressEvent(event);
        }
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::scaleImage(double newScale)
{
    double newBoundedScale = qBound(0.1, newScale, 10.0);

    if (qFuzzyCompare(scaleFactor, newBoundedScale)) {
        return;
    }

    double factor = newBoundedScale / scaleFactor;
    ui->graphicsView->scale(factor, factor);
    scaleFactor = newBoundedScale;
    ui->statusbar->showMessage(QString("缩放比例: %1%").arg(int(scaleFactor * 100)));
}

void MainWindow::fitToWindow()
{
    if (!pixmapItem) return;

    ui->graphicsView->fitInView(imageScene->sceneRect(), Qt::KeepAspectRatio);
    scaleFactor = ui->graphicsView->transform().m11();
}

void MainWindow::updateImageInfo()
{
    if (currentFilePath.isEmpty() || originalPixmap.isNull()) {
        ui->imageName->setText("图片名称");
        ui->imageFormat->setText("图片格式");
        ui->imageResolution->setText("图片分辨率");
        ui->imageSize->setText("图片大小");
        ui->imageRGBColour->setText("RGB颜色");
        ui->imageSaturation->setText("饱和度");
        return;
    }

    QFileInfo fileInfo(currentFilePath);
    ui->imageName->setText(fileInfo.fileName());
    ui->imageFormat->setText(fileInfo.suffix().toUpper());
    ui->imageResolution->setText(QString("%1 x %2").arg(originalPixmap.width()).arg(originalPixmap.height()));

    qint64 size = fileInfo.size();
    QString sizeString;
    if (size > 1024 * 1024)
        sizeString = QString::asprintf("%.2f MB", size / (1024.0 * 1024.0));
    else if (size > 1024)
        sizeString = QString::asprintf("%.2f KB", size / 1024.0);
    else
        sizeString = QString("%1 Bytes").arg(size);
    ui->imageSize->setText(sizeString);

    ui->imageRGBColour->setText("RGB颜色");
    ui->imageSaturation->setText("饱和度");
}
