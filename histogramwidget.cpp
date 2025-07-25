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
// File: histogramwidget.cpp
//
// Description:
// HistogramWidget 类的实现文件。该文件包含了计算和绘制图像直方图
// 的具体逻辑。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include "histogramwidget.h"
#include <QPainter>

/**
 * @brief HistogramWidget 构造函数。
 *
 * 初始化所有通道的直方图数据容器，将其大小设置为256并填充为0。
 * @param parent 父窗口部件。
 */
HistogramWidget::HistogramWidget(QWidget *parent)
    : QWidget(parent), isGrayscale(false)
{
    // 初始化存储直方图数据的QVector，每个通道有256个bin（对应0-255的像素值）
    redChannel.fill(0, 256);
    greenChannel.fill(0, 256);
    blueChannel.fill(0, 256);
    grayChannel.fill(0, 256);
}

/**
 * @brief 公共接口，用于更新并显示新图像的直方图。
 *
 * 如果图像为空，则清空直方图数据；否则，调用私有方法计算新数据。
 * 最后，调用 update() 来触发一次重绘事件（paintEvent）。
 * @param image 要分析的图像。
 */
void HistogramWidget::updateHistogram(const QImage &image)
{
    if (image.isNull()) {
        // 如果传入的图像为空，则重置所有直方图数据
        redChannel.fill(0, 256);
        greenChannel.fill(0, 256);
        blueChannel.fill(0, 256);
        grayChannel.fill(0, 256);
        update(); // 触发重绘以清空显示
        return;
    }
    // 计算新图像的直方图
    calculateHistogram(image);
    // 触发重绘以显示新的直方图
    update();
}

/**
 * @brief 计算并存储给定图像的直方图数据。
 *
 * 遍历图像的每一个像素，根据图像是灰度图还是彩色图，
 * 统计每个强度级别（0-255）的像素数量。
 * @param image 要分析的图像。
 */
void HistogramWidget::calculateHistogram(const QImage &image)
{
    // 在计算前先清空旧数据
    redChannel.fill(0, 256);
    greenChannel.fill(0, 256);
    blueChannel.fill(0, 256);
    grayChannel.fill(0, 256);

    isGrayscale = image.isGrayscale();

    if (isGrayscale) {
        // 处理灰度图
        for (int y = 0; y < image.height(); ++y) {
            const uchar *scanLine = image.constScanLine(y);
            for (int x = 0; x < image.width(); ++x) {
                // scanLine[x] 直接就是像素的灰度值
                grayChannel[scanLine[x]]++;
            }
        }
    } else {
        // 处理彩色图 (RGB/ARGB)
        for (int y = 0; y < image.height(); ++y) {
            // 将每行像素的指针转换为QRgb类型指针
            const QRgb *scanLine = reinterpret_cast<const QRgb*>(image.constScanLine(y));
            for (int x = 0; x < image.width(); ++x) {
                // 使用Qt的辅助函数提取R, G, B分量
                redChannel[qRed(scanLine[x])]++;
                greenChannel[qGreen(scanLine[x])]++;
                blueChannel[qBlue(scanLine[x])]++;
            }
        }
    }
}

/**
 * @brief 重写的绘制事件处理器。
 *
 * 负责将存储的直方图数据可视化地绘制出来。
 * @param event 绘制事件。
 */
void HistogramWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    // 绘制背景
    painter.fillRect(rect(), Qt::lightGray);

    if (isGrayscale) {
        // --- 绘制灰度直方图 ---
        // 1. 找到直方图中的最大值，用于归一化高度
        int maxVal = 0;
        for (int count : grayChannel) {
            if (count > maxVal) {
                maxVal = count;
            }
        }
        if (maxVal == 0) return; // 如果图像为空或纯黑，则不绘制

        // 2. 绘制直方图
        painter.setPen(Qt::white);
        float barWidth = (float)width() / 256.0;
        for (int i = 0; i < 256; ++i) {
            float barHeight = (float)height() * grayChannel[i] / maxVal;
            // 从底部向上绘制直线代表每个bin的高度
            painter.drawLine(i * barWidth, height(), i * barWidth, height() - barHeight);
        }
    } else {
        // --- 绘制彩色直方图 ---
        // 1. 找到所有通道中的最大值，以确保所有通道使用相同的比例
        int maxVal = 0;
        for (int i = 0; i < 256; ++i) {
            maxVal = qMax(maxVal, redChannel[i]);
            maxVal = qMax(maxVal, greenChannel[i]);
            maxVal = qMax(maxVal, blueChannel[i]);
        }
        if (maxVal == 0) return;

        float barWidth = (float)width() / 256.0;
        // 设置半透明效果，以便观察重叠的通道
        painter.setOpacity(0.7);

        // 2. 绘制红色通道
        painter.setPen(Qt::red);
        for (int i = 0; i < 256; ++i) {
            float barHeight = (float)height() * redChannel[i] / maxVal;
            painter.drawLine(i * barWidth, height(), i * barWidth, height() - barHeight);
        }

        // 3. 绘制绿色通道
        painter.setPen(Qt::green);
        for (int i = 0; i < 256; ++i) {
            float barHeight = (float)height() * greenChannel[i] / maxVal;
            painter.drawLine(i * barWidth, height(), i * barWidth, height() - barHeight);
        }

        // 4. 绘制蓝色通道
        painter.setPen(Qt::blue);
        for (int i = 0; i < 256; ++i) {
            float barHeight = (float)height() * blueChannel[i] / maxVal;
            painter.drawLine(i * barWidth, height(), i * barWidth, height() - barHeight);
        }
    }
}
