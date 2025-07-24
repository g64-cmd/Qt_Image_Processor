#ifndef DRAGGABLEITEMMODEL_H
#define DRAGGABLEITEMMODEL_H

#include <QStandardItemModel>

class QMimeData;

/**
 * @brief 一个支持拖拽操作的数据模型
 *
 * 重写了 flags(), mimeTypes(), 和 mimeData() 函数，
 * 以便在拖动时能够正确地打包自定义数据。
 */
class DraggableItemModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit DraggableItemModel(QObject *parent = nullptr);
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
};

#endif // DRAGGABLEITEMMODEL_H
