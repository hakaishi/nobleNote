#include "findfilemodel.h"
#include <QFileIconProvider>

FindFileModel::FindFileModel(QObject *parent) :
    QStandardItemModel(parent)
{
}

QString FindFileModel::fileName(const QModelIndex &index) const
{
    return QFileInfo(itemFromIndex(index)->data(Qt::UserRole + 1).toString()).fileName();
}

QString FindFileModel::filePath(const QModelIndex &index) const
{
    return QFileInfo(itemFromIndex(index)->data(Qt::UserRole + 1).toString()).filePath();
}

bool FindFileModel::remove(const QModelIndex &index) const
{
    return QFile::remove(itemFromIndex(index)->data(Qt::UserRole + 1).toString());
}

QFileInfo FindFileModel::fileInfo(const QModelIndex &index) const
{
    return QFileInfo(itemFromIndex(index)->data(Qt::UserRole + 1).toString());
}

void FindFileModel::appendFile(QString file)
{
    QFileInfo info(file);
    if(info.path().isEmpty() || info.path() == ".")
    {
        qWarning("FindFileModel::appendFile failed: file name must contain the full path");
        return;
    }


    QStandardItem * fileItem = new QStandardItem(info.fileName());
    fileItem->setIcon(QFileIconProvider().icon(info));
    fileItem->setData(file,Qt::UserRole + 1); // store as user data
    appendRow(fileItem);
}
