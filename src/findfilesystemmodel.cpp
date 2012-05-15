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
     qDebug("FindFileSystemModel::fileName failed: cast failed");
    return QString();
}

QString FindFileSystemModel::filePath(const QModelIndex &index) const
{
    if(QFileSystemModel* fsm= qobject_cast<QFileSystemModel*>(sourceModel()))
      return fsm->filePath(mapToSource(index));
    else if(FindFileModel* ffm= qobject_cast<FindFileModel*>(sourceModel()))
      return ffm->filePath(mapToSource(index));
     qDebug("FindFileSystemModel::filePath failed: cast failed");
    return QString();
}

bool FindFileSystemModel::remove(const QModelIndex &index) const
{
    if(QFileSystemModel* fsm= qobject_cast<QFileSystemModel*>(sourceModel()))
      return fsm->remove(mapToSource(index));
    else if(FindFileModel* ffm= qobject_cast<FindFileModel*>(sourceModel()))
      return ffm->remove(mapToSource(index));
     qDebug("FindFileSystemModel::remove failed: cast failed");
    return false;
}

QFileInfo FindFileSystemModel::fileInfo(const QModelIndex &index) const
{
    if(QFileSystemModel* fsm= qobject_cast<QFileSystemModel*>(sourceModel()))
      return fsm->fileInfo(mapToSource(index));
    else if(FindFileModel* ffm= qobject_cast<FindFileModel*>(sourceModel()))
      return ffm->fileInfo(mapToSource(index));
     qDebug("FindFileSystemModel::fileInfo failed : cast failed");
    return QFileInfo();
}

void FindFileSystemModel::appendFile(QString filePath)
{
    if(FindFileModel* ffm= qobject_cast<FindFileModel*>(sourceModel()))
      ffm->appendFile(filePath);
    else
     qDebug("FindFileSystemModel::appendFile failed: cast failed");
}

QModelIndex FindFileSystemModel::setRootPath(const QString &newPath)
{
    if(QFileSystemModel* fsm= qobject_cast<QFileSystemModel*>(sourceModel()))
      return mapFromSource(fsm->setRootPath(newPath));
    qDebug("FindFileSystemModel::setRootPath failed: cast failed");
    return QModelIndex();
}

QString FindFileSystemModel::rootPath() const
{
    if(QFileSystemModel* fsm= qobject_cast<QFileSystemModel*>(sourceModel()))
      return fsm->rootPath();
    qDebug("FindFileSystemModel::rootPath failed: cast failed");
    return QString();
}

void FindFileSystemModel::clear()
{
    if(FindFileModel* ffm= qobject_cast<FindFileModel*>(sourceModel()))
        ffm->clear();
    else
        qDebug("FindFileSystemModel::clear failed: cast failed");
}

QStringList FindFileSystemModel::find(QString searchName, QString searchText, QString path)
{
    if(FindFileModel* ffm= qobject_cast<FindFileModel*>(sourceModel()))
        return ffm->find(searchName,searchText,path);
   qDebug("FindFileSystemModel::find failed: cast failed");
   return QStringList();
}
