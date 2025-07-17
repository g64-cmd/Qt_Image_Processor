#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("图像视频处理软件 v1.0"); // 设置窗口标题
    w.show();
    return a.exec();
}
