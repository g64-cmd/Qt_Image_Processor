#include "histogramwidget.h"
#include <QPainter>

HistogramWidget::HistogramWidget(QWidget *parent)
    : QWidget(parent), isGrayscale(false)
{
    redChannel.fill(0, 256);
    greenChannel.fill(0, 256);
    blueChannel.fill(0, 256);
    grayChannel.fill(0, 256);
}

void HistogramWidget::updateHistogram(const QImage &image)
{
    if (image.isNull()) {
        redChannel.fill(0, 256);
        greenChannel.fill(0, 256);
        blueChannel.fill(0, 256);
        grayChannel.fill(0, 256);
        update();
        return;
    }
    calculateHistogram(image);
    update();
}

void HistogramWidget::calculateHistogram(const QImage &image)
{
    redChannel.fill(0, 256);
    greenChannel.fill(0, 256);
    blueChannel.fill(0, 256);
    grayChannel.fill(0, 256);
    isGrayscale = image.isGrayscale();
    if (isGrayscale) {
        for (int y = 0; y < image.height(); ++y) {
            const uchar *scanLine = image.constScanLine(y);
            for (int x = 0; x < image.width(); ++x) {
                grayChannel[scanLine[x]]++;
            }
        }
    } else {
        for (int y = 0; y < image.height(); ++y) {
            const QRgb *scanLine = reinterpret_cast<const QRgb*>(image.constScanLine(y));
            for (int x = 0; x < image.width(); ++x) {
                redChannel[qRed(scanLine[x])]++;
                greenChannel[qGreen(scanLine[x])]++;
                blueChannel[qBlue(scanLine[x])]++;
            }
        }
    }
}

void HistogramWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), Qt::lightGray);

    if (isGrayscale) {
        int maxVal = 0;
        for (int count : grayChannel) {
            if (count > maxVal) {
                maxVal = count;
            }
        }
        if (maxVal == 0) return;

        painter.setPen(Qt::white);
        float barWidth = (float)width() / 256.0;
        for (int i = 0; i < 256; ++i) {
            float barHeight = (float)height() * grayChannel[i] / maxVal;
            painter.drawLine(i * barWidth, height(), i * barWidth, height() - barHeight);
        }
    } else {
        int maxVal = 0;
        for (int i = 0; i < 256; ++i) {
            maxVal = qMax(maxVal, redChannel[i]);
            maxVal = qMax(maxVal, greenChannel[i]);
            maxVal = qMax(maxVal, blueChannel[i]);
        }
        if (maxVal == 0) return;
        float barWidth = (float)width() / 256.0;
        painter.setOpacity(0.7);
        painter.setPen(Qt::red);
        for (int i = 0; i < 256; ++i) {
            float barHeight = (float)height() * redChannel[i] / maxVal;
            painter.drawLine(i * barWidth, height(), i * barWidth, height() - barHeight);
        }
        painter.setPen(Qt::green);
        for (int i = 0; i < 256; ++i) {
            float barHeight = (float)height() * greenChannel[i] / maxVal;
            painter.drawLine(i * barWidth, height(), i * barWidth, height() - barHeight);
        }
        painter.setPen(Qt::blue);
        for (int i = 0; i < 256; ++i) {
            float barHeight = (float)height() * blueChannel[i] / maxVal;
            painter.drawLine(i * barWidth, height(), i * barWidth, height() - barHeight);
        }
    }
}
