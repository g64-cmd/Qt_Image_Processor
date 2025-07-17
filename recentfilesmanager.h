#ifndef RECENTFILESMANAGER_H
#define RECENTFILESMANAGER_H

#include <QObject>
#include <QStringList>

class QStandardItemModel;

/**
 * @brief 近期文件列表管理器
 *
 * 封装了管理近期文件列表的所有逻辑，包括数据存储和UI模型更新。
 */
class RecentFilesManager : public QObject
{
    Q_OBJECT
public:
    explicit RecentFilesManager(QStandardItemModel *model, QObject *parent = nullptr);

    /**
     * @brief 添加一个新文件到列表顶部
     * @param filePath 文件的完整路径
     */
    void addFile(const QString &filePath);

    /**
     * @brief 获取当前所有近期文件的路径
     * @return 包含文件路径的字符串列表
     */
    const QStringList& getRecentFilePaths() const;

private:
    void updateModel(); // 私有辅助函数，用于刷新UI模型

    QStandardItemModel *model; // 指向UI模型的指针 (不拥有所有权)
    QStringList recentFilePaths;
    static const int MaxRecentFiles = 10;
};

#endif // RECENTFILESMANAGER_H
