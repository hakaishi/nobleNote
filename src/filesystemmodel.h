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
    explicit FileSystemModel(QObject *parent = 0) : QFileSystemModel(parent), onlyOnItemDrops_(false) {}
    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        return QFileSystemModel::flags(index) | Qt::ItemIsEditable;
    }

    // disable drops between items and elsewhere in the viewport
    void setOnlyOnItemDrops(bool b){onlyOnItemDrops_ = b; }
    bool onlyOnItemDrops() const { return onlyOnItemDrops_;}

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
    {
        if(!onlyOnItemDrops_ || (row == -1 && column == -1)) // dropped directly on parent
            return QFileSystemModel::dropMimeData(data,action,row,column,parent);
        return false; // dropped between items
    }

    bool onlyOnItemDrops_;
};

#endif // FILESYSTEMMODEL_H
