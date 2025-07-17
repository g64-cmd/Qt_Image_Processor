#include "recentfilesmanager.h"
#include <QStandardItemModel>
#include <QIcon>
#include <QFileInfo>

RecentFilesManager::RecentFilesManager(QStandardItemModel *model, QObject *parent)
    : QObject(parent), model(model)
{
    // 确保传入的模型是有效的
    Q_ASSERT(model != nullptr);
}

void RecentFilesManager::addFile(const QString &filePath)
{
    // 从列表中移除已存在的相同路径，以确保新打开的在最前面
    recentFilePaths.removeAll(filePath);
    // 将新路径添加到列表的开头
    recentFilePaths.prepend(filePath);
    // 如果列表超过最大数量，则移除末尾的项
    while (recentFilePaths.size() > MaxRecentFiles) {
        recentFilePaths.removeLast();
    }

    // 更新UI模型
    updateModel();
}

const QStringList& RecentFilesManager::getRecentFilePaths() const
{
    return recentFilePaths;
}

void RecentFilesManager::updateModel()
{
    if (!model) return;

    model->clear(); // 清空旧模型
    for (const QString &path : qAsConst(recentFilePaths)) {
        QFileInfo fileInfo(path);
        QPixmap thumbnail(path);

        QStandardItem *item = new QStandardItem();
        item->setIcon(QIcon(thumbnail.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        item->setText(fileInfo.fileName());
        item->setData(path, Qt::UserRole); // 将完整路径存储在项中

        model->appendRow(item);
    }
}
