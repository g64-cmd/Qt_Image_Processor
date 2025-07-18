#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QFontDatabase>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // --- 1. 加载自定义字体 ---
    // 路径已更新为匹配 .qrc 文件中的前缀和您的确切文件名
    int fontId = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/Inter_18pt-Regular.ttf");
    if (fontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.empty()) {
            QFont defaultFont(fontFamilies.at(0), 13);
            a.setFont(defaultFont);
        }
    } else {
        qWarning() << "Could not load custom font: :/fonts/resources/fonts/Inter_18pt-Regular.ttf";
    }

    // --- 2. 加载全局样式表 ---
    // 路径已更新为匹配 .qrc 文件中的前缀
    QFile styleFile(":/styles/resources/styles/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        a.setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        qWarning() << "Could not open stylesheet file: :/styles/resources/styles/style.qss";
    }

    MainWindow w;
    w.setWindowTitle("图像视频处理软件 v1.0");
    w.show();
    return a.exec();
}
