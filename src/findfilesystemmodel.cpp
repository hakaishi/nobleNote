/* nobleNote, a note taking application
 * Copyright (C) 2020 Christian Metscher <hakaishi@web.de>,
                      Fabian Deuchler <Taiko000@gmail.com>

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.

 * nobleNote is licensed under the MIT, see `http://copyfree.org/licenses/mit/license.txt'.
 */

#include "findfilesystemmodel.h"
#include "findfilemodel.h"
#include "filesystemmodel.h"
#include <QFileSystemModel>
#include "htmlnotereader.h"

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

bool FindFileSystemModel::allSizeZero(const QList<QModelIndex> &indices) const
{
    for(QModelIndex idx : indices)
    {
        int size = 0;
        if(QFileSystemModel* fsm= qobject_cast<QFileSystemModel*>(sourceModel()))
        {
            // requires QFileInfo wrapper, because QFileSystemModel::size only updates after app restart
            size = QFileInfo(fsm->filePath(mapToSource(idx))).size();
        }
        else if(FindFileModel* ffm= qobject_cast<FindFileModel*>(sourceModel()))
        {
            size = ffm->size(mapToSource(idx));
        }

        if(size > 0) // at least one non-zero
        {
            return false;
        }
    }
    return true;
}

QStringList FindFileSystemModel::fileNames(const QList<QModelIndex> &indices) const
{
    QHash<QModelIndex,QString> names;
    for(QModelIndex idx : indices)
    {
         if(QFileSystemModel* fsm= qobject_cast<QFileSystemModel*>(sourceModel()))
           names.insert(idx,fsm->fileName(mapToSource(idx)));
         else if(FindFileModel* ffm= qobject_cast<FindFileModel*>(sourceModel()))
           names.insert(idx,ffm->fileName(mapToSource(idx)));
    }
    return names.values();
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

bool FindFileSystemModel::rmdir(const QModelIndex &index) const
{
    if(QFileSystemModel* fsm= qobject_cast<QFileSystemModel*>(sourceModel()))
      return fsm->rmdir(mapToSource(index));
    qDebug("FindFileSystemModel::rmdir failed: cast failed. This method is only implemented for QFileSystemModel");
    return false;
}

QModelIndex FindFileSystemModel::mkdir(const QModelIndex &parent, const QString &name)
{
    if(QFileSystemModel* fsm= qobject_cast<QFileSystemModel*>(sourceModel()))
      return mapFromSource(fsm->mkdir(mapToSource(parent),name));
    qDebug("FindFileSystemModel::mkdir failed: cast failed. This method is only implemented for QFileSystemModel");
    return QModelIndex();
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

bool FindFileSystemModel::removeList(const QModelIndexList &index) const
{
    bool successfull = true;
    for(QModelIndex idx : index)
    {
        if(QFileSystemModel* fsm= qobject_cast<QFileSystemModel*>(sourceModel()))
        {
            if(!fsm->remove(mapToSource(idx)))
                successfull = false;
        }
        else if(FindFileModel* ffm= qobject_cast<FindFileModel*>(sourceModel()))
        {
                if(!ffm->remove(mapToSource(idx)))
                    successfull = false;
        }
    }
    if(!successfull)
      qDebug("FindFileSystemModel::remove failed: one or more files could not be removed");

    return successfull;
}

/*static*/ bool FindFileSystemModel::removeList(const QFileInfoList &fileInfos)
{
    qDebug("FindFileSystemModel::removeList FIXME: removal works?");
    bool successfull = true;
    for(const QFileInfo& fileInfo : fileInfos)
    {
        QString path = fileInfo.path();
        if(!QFile::remove(fileInfo.path()))
            successfull = false;
    }
    return successfull;
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

void FindFileSystemModel::findInFiles(const QString &fileName, const QString &content, const QString &path)
{
    if(FindFileModel* ffm= qobject_cast<FindFileModel*>(sourceModel()))
    {
        ffm->findInFiles(fileName,content,path);
        return;
    }
    qDebug("FindFileSystemModel::findInFiles failed: cast failed");
}

QModelIndex FindFileSystemModel::index(const QString &path, int column) const
{
    if(QFileSystemModel* fsm= qobject_cast<QFileSystemModel*>(sourceModel()))
        return mapFromSource(fsm->index(path,column));
    qDebug("FindFileSystemModel::index failed: cast failed. This method is only implemented for QFileSystemModel");
    return QModelIndex();
}

void FindFileSystemModel::copyNotesToBackupDir(const QModelIndexList& indexes) const
{
    for(const QModelIndex& index : indexes)
    {
        QString filePath = this->filePath(index);
        QUuid uuid = HtmlNoteReader::uuid(filePath);
        if(!uuid.isNull())
            QFile::copy(filePath, QSettings().value("backup_dir_path").toString() + QDir::separator() + uuid.toString().mid(1,36));
    }
}

/*static*/ void FindFileSystemModel::copyNotesToBackupDir(const QFileInfoList& fileInfos)
{
    for(const QFileInfo& fileInfo : fileInfos)
    {
        QUuid uuid = HtmlNoteReader::uuid(fileInfo.path());
        if(!uuid.isNull())
            QFile::copy(fileInfo.path(), QSettings().value("backup_dir_path").toString() + QDir::separator() + uuid.toString().mid(1,36));
    }
}
