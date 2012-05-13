#include "findfilesystemmodel.h"
#include "findfilemodel.h"
#include <QFileSystemModel>
#include "filesystemmodel.h"

FindFileSystemModel::FindFileSystemModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

QString FindFileSystemModel::fileName(const QModelIndex &index) const
{
    if(QFileSystemModel* fsm= qobject_cast<QFileSystemModel*>(sourceModel()))
            return fsm->fileName(mapToSource(index));
    else if(FindFileModel* ffm= qobject_cast<FindFileModel*>(sourceModel()))
        return ffm->fileName(mapToSource(index));
     qDebug("FindFileSystemModel: cast failed");
    return QString();
}

QString FindFileSystemModel::filePath(const QModelIndex &index) const
{
    if(QFileSystemModel* fsm= qobject_cast<QFileSystemModel*>(sourceModel()))
            return fsm->filePath(mapToSource(index));
    else if(FindFileModel* ffm= qobject_cast<FindFileModel*>(sourceModel()))
        return ffm->filePath(mapToSource(index));
     qDebug("FindFileSystemModel: cast failed");
    return QString();
}

bool FindFileSystemModel::remove(const QModelIndex &index) const
{
    if(QFileSystemModel* fsm= qobject_cast<QFileSystemModel*>(sourceModel()))
            return fsm->remove(mapToSource(index));
    else if(FindFileModel* ffm= qobject_cast<FindFileModel*>(sourceModel()))
        return ffm->remove(mapToSource(index));
     qDebug("FindFileSystemModel: cast failed");
    return false;
}

QFileInfo FindFileSystemModel::fileInfo(const QModelIndex &index) const
{
    if(QFileSystemModel* fsm= qobject_cast<QFileSystemModel*>(sourceModel()))
            return fsm->fileInfo(mapToSource(index));
    else if(FindFileModel* ffm= qobject_cast<FindFileModel*>(sourceModel()))
        return ffm->fileInfo(mapToSource(index));
     qDebug("FindFileSystemModel: cast failed");
    return QFileInfo();
}

void FindFileSystemModel::appendFile(QString file)
{
    if(FindFileModel* ffm= qobject_cast<FindFileModel*>(sourceModel()))
            return ffm->appendFile(file);
     qDebug("FindFileSystemModel: cast failed");
}

QModelIndex FindFileSystemModel::setRootPath(const QString &newPath)
{
    if(QFileSystemModel* fsm= qobject_cast<QFileSystemModel*>(sourceModel()))
            return mapFromSource(fsm->setRootPath(newPath));
    qDebug("FindFileSystemModel: cast failed");
    return QModelIndex();
}

QString FindFileSystemModel::rootPath() const
{
    if(QFileSystemModel* fsm= qobject_cast<QFileSystemModel*>(sourceModel()))
            return fsm->rootPath();
    qDebug("FindFileSystemModel: cast failed");
    return QString();
}
