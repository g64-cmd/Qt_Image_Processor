#ifndef DRAGGABLEITEMMODEL_H
#define DRAGGABLEITEMMODEL_H

#include <QStandardItemModel>

/**
 * @brief 一个可拖拽的模型
 *
 * 重写了 mimeData 函数，以便在拖动时打包自定义数据。
 */
class DraggableItemModel : public QStandardItemModel
{
public:
    explicit DraggableItemModel(QObject *parent = nullptr);

    // 重写此函数是解决拖放问题的关键
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
};

#endif // DRAGGABLEITEMMODEL_H
