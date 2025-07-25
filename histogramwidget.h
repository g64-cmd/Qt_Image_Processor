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

#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

// =============================================================================
// File: histogramwidget.h
//
// Description:
// 该文件定义了 HistogramWidget 类，这是一个自定义的 QWidget，
// 用于计算和绘制图像的色彩直方图。
//
// Author: g64
// Date: 2025-07-25
// =============================================================================

#include <QWidget>
#include <QImage>
#include <QVector>

/**
 * @class HistogramWidget
 * @brief 一个用于显示图像色彩直方图的自定义控件。
 *
 * 该控件可以接收一个 QImage，计算其RGB三通道或灰度通道的直方图，
 * 并通过重写 paintEvent 将其可视化地绘制出来。
 */
class HistogramWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     * @param parent 父窗口部件，默认为nullptr。
     */
    explicit HistogramWidget(QWidget *parent = nullptr);

    /**
     * @brief 公共接口，用于更新并显示新图像的直方图。
     * @param image 要分析的图像。
     */
    void updateHistogram(const QImage &image);

protected:
    /**
     * @brief 重写的绘制事件处理器。
     *
     * 当控件需要重绘时，此函数被调用，负责将存储的直方图数据绘制到界面上。
     * @param event 绘制事件。
     */
    void paintEvent(QPaintEvent *event) override;

private:
    /**
     * @brief 计算并存储给定图像的直方图数据。
     * @param image 要分析的图像。
     */
    void calculateHistogram(const QImage &image);

    // --- 成员变量 ---
    QVector<int> redChannel;    // 存储红色通道的直方图数据
    QVector<int> greenChannel;  // 存储绿色通道的直方图数据
    QVector<int> blueChannel;   // 存储蓝色通道的直方图数据
    QVector<int> grayChannel;   // 存储灰度通道的直方图数据
    bool isGrayscale;           // 标记当前处理的图像是否为灰度图
};

#endif // HISTOGRAMWIDGET_H
