#ifndef FINDFILESYSTEMMODEL_H
#define FINDFILESYSTEMMODEL_H

#include <QSortFilterProxyModel>
#include <QFileInfo>

/**
 * @brief  proxy model for both QFileSystemModel and for FindFileModel
 *          that allows access to some methods that exist in both classes
 *          call setSourceModel to an instance of one of these models,
 *          the sourceModel() will be automatically cast to one of these two models
 *
 *          Important: always use mapToSource and mapFromSource for QModelIndexes
 */

class FindFileSystemModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit FindFileSystemModel(QObject *parent = 0);
    QString fileName(const QModelIndex & index) const;
    QString filePath(const QModelIndex & index) const;
    bool remove(const QModelIndex & index ) const;
    QFileInfo fileInfo(const QModelIndex & index) const;
    void appendFile(QString filePath); // append file with full path
    QModelIndex setRootPath(const QString & newPath);
    QString rootPath() const;
    void clear();
    QStringList find0(QString searchName, QString searchText, QString path);
    void find(const QString &text,const QString& path);

    // TODO signal file changed
};

#endif // FINDFILESYSTEMMODEL_H
