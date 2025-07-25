// =============================================================================
//
// Copyright (C) 2025 g64-cmd
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// =============================================================================

// =============================================================================
// File: main.cpp
//
// Description:
// 应用程序的入口点。
// 该文件包含 main 函数，负责：
// - 创建 QApplication 实例。
// - 加载并应用全局字体和样式表 (QSS)。
// - 创建并显示主窗口 (MainWindow)。
// - 启动Qt事件循环。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QFontDatabase>
#include <QDebug>
#include <QTranslator>

/**
 * @brief 应用程序的主函数入口。
 * @param argc 命令行参数计数。
 * @param argv 命令行参数数组。
 * @return 应用程序退出码。
 */
int main(int argc, char *argv[])
{
    // --- 1. 应用程序初始化 ---
    // 创建QApplication实例，它是所有Qt GUI应用程序的核心。
    QApplication a(argc, argv);

    // --- 2. 加载全局字体 ---
    // 从资源文件加载自定义字体，并将其设置为应用程序的默认字体。
    // 这可以确保整个应用的UI具有统一的字体风格。
    int fontId = QFontDatabase::addApplicationFont(":/fonts/resources/fonts/Inter_18pt-Regular.ttf");
    if (fontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.empty()) {
            QFont defaultFont(fontFamilies.at(0), 13); // 使用加载的字体，设置默认大小
            a.setFont(defaultFont);
        }
    } else {
        qWarning() << "Could not load custom font: :/fonts/resources/fonts/Inter_18pt-Regular.ttf";
    }

    // --- 3. 加载全局样式表 (QSS) ---
    // 从资源文件加载.qss样式表，并将其应用到整个应用程序。
    // 这允许我们像Web开发中的CSS一样，集中管理UI控件的外观。
    QFile styleFile(":/styles/resources/styles/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        a.setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        qWarning() << "Could not open stylesheet file: :/styles/resources/styles/style.qss";
    }

    // --- 4. 加载翻译文件 ---
    // 为了支持国际化，加载中文翻译文件。
    QTranslator translator;
    if (translator.load(":/translations/Qt_Image_Processor_zh_CN.qm")) {
        a.installTranslator(&translator);
    } else {
        qWarning() << "Could not load translation file: :/translations/Qt_Image_Processor_zh_CN.qm";
    }

    // --- 5. 创建并显示主窗口 ---
    MainWindow w;
    w.setWindowTitle(QObject::tr("图像视频处理软件 v1.0")); // 使用tr()以支持翻译
    w.show(); // 显示主窗口

    // --- 6. 启动事件循环 ---
    // a.exec() 会进入Qt的主事件循环，开始处理用户输入、窗口重绘等事件。
    // 程序将在此处阻塞，直到应用程序退出。
    return a.exec();
}
