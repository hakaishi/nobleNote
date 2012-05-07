#ifndef FILESYSTEMMODEL_H
#define FILESYSTEMMODEL_H

#include <QFileSystemModel>

/**
 * @brief  overwritten base class that circumvents a bug in QFileSystemodel which causes
 * the flags method to never return a Qt::ItemIsEditable
 *  because it checks for the permission QFile::WriteUser which seems to be always false
 *  if Qt::ItemIsEditable is never set, setData or edit methods will always fail
 */

class FileSystemModel : public QFileSystemModel
{
    Q_OBJECT
public:
    explicit FileSystemModel(QObject *parent = 0);
    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        return QFileSystemModel::flags(index) | Qt::ItemIsEditable;
    }
};

#endif // FILESYSTEMMODEL_H
