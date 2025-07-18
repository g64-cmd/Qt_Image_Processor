#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QWidget>
#include <QImage>
#include <QVector>

class HistogramWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HistogramWidget(QWidget *parent = nullptr);

    // 更新直方图数据
    void updateHistogram(const QImage &image);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void calculateHistogram(const QImage &image);

    QVector<int> redChannel;
    QVector<int> greenChannel;
    QVector<int> blueChannel;
    QVector<int> grayChannel;
    bool isGrayscale;
};

#endif // HISTOGRAMWIDGET_H
