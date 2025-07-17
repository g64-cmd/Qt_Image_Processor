#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QImageReader>
#include <QWheelEvent>
#include <QGuiApplication>
#include <QFileInfo>
#include <QtMath>
#include <QPainter> // 用于设置渲染提示

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scaleFactor(1.0)
    , imageScene(nullptr) // 初始化指针
    , pixmapItem(nullptr)
{
    // 设置UI
    ui->setupUi(this);

    // --- 使用 QGraphicsView 进行设置 ---

    // 1. 创建场景
    imageScene = new QGraphicsScene(this);

    // 2. 为 graphicsView (在 .ui 文件中定义) 设置场景
    ui->graphicsView->setScene(imageScene);

    // 3. 设置拖动模式为“手掌拖动”，这是实现平移的关键！
    //    当鼠标在视图上按下并拖动时，视图内容会随之滚动。
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    // 4. 设置高质量的渲染，使缩放后的图像更平滑
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);

    // 5. 设置视图的对齐方式和更新模式，以获得更好的性能和外观
    ui->graphicsView->setAlignment(Qt::AlignCenter);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    // 6. 在视图的视口上安装事件过滤器，以捕获滚轮事件用于缩放
    ui->graphicsView->viewport()->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionopen_triggered()
{
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
        return; // 加载失败则提前返回
    }

    // --- 使用 QGraphicsView 显示图像 ---

    // 1. 清空上一次的场景内容
    imageScene->clear();

    // 2. 将 QPixmap 添加到场景中，返回一个 QGraphicsPixmapItem 指针
    pixmapItem = imageScene->addPixmap(originalPixmap);

    // 3. 设置场景的边界矩形，与图像大小一致
    imageScene->setSceneRect(originalPixmap.rect());

    // 4. 调整图像以适应窗口大小
    fitToWindow();

    // 更新底部的图片信息
    updateImageInfo();

    ui->statusbar->showMessage(tr("成功打开: %1").arg(currentFilePath), 5000);
}

/**
 * @brief 事件过滤器，现在只用于处理 graphicsView 上的滚轮缩放事件
 */
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // 确保事件来自 graphicsView 的视口，并且场景中有图像
    if (watched == ui->graphicsView->viewport() && event->type() == QEvent::Wheel && pixmapItem) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        int angle = wheelEvent->angleDelta().y();

        // 根据滚轮方向计算缩放因子
        double zoomFactor = (angle > 0) ? 1.15 : (1.0 / 1.15);

        // 调用 scaleImage 函数进行缩放
        scaleImage(scaleFactor * zoomFactor);

        return true; // 事件已处理，不再向下传递
    }

    // 将其他事件传递给基类
    return QMainWindow::eventFilter(watched, event);
}

/**
 * @brief 键盘按下事件，用于处理Ctrl+和Ctrl-快捷键缩放
 */
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (!pixmapItem) { // 检查 pixmapItem 是否有效
        QMainWindow::keyPressEvent(event);
        return;
    }

    if (event->modifiers() == Qt::ControlModifier) {
        if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
            scaleImage(scaleFactor * 1.2); // 放大
        } else if (event->key() == Qt::Key_Minus) {
            scaleImage(scaleFactor * 0.8); // 缩小
        } else {
            QMainWindow::keyPressEvent(event);
        }
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

/**
 * @brief 根据给定的因子缩放图像
 * @param newScale 新的绝对缩放比例
 */
void MainWindow::scaleImage(double newScale)
{
    // 限制缩放范围
    double newBoundedScale = qBound(0.1, newScale, 10.0);

    if (qFuzzyCompare(scaleFactor, newBoundedScale)) {
        return; // 如果缩放比例没有变化，则不执行任何操作
    }

    // 计算相对于当前缩放比例的因子
    double factor = newBoundedScale / scaleFactor;

    // QGraphicsView::scale 会在当前变换的基础上进行缩放
    ui->graphicsView->scale(factor, factor);

    // 更新我们自己记录的缩放比例
    scaleFactor = newBoundedScale;

    ui->statusbar->showMessage(QString("缩放比例: %1%").arg(int(scaleFactor * 100)));
}


/**
 * @brief 使图像适应窗口大小进行显示
 */
void MainWindow::fitToWindow()
{
    if (!pixmapItem) return;

    // fitInView 会自动计算并应用合适的变换，使场景内容完整地显示在视图中
    ui->graphicsView->fitInView(imageScene->sceneRect(), Qt::KeepAspectRatio);

    // 更新我们记录的缩放因子，m11() 是水平缩放，m22() 是垂直缩放
    // 对于等比缩放，它们是相等的。
    scaleFactor = ui->graphicsView->transform().m11();
}


/**
 * @brief 更新底部面板的图像信息
 */
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

    // 其他信息暂时作为占位符
    ui->imageRGBColour->setText("RGB颜色");
    ui->imageSaturation->setText("饱和度");
}
