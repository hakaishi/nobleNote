/* nobleNote, a note taking application
 * Copyright (C) 2015 Christian Metscher <hakaishi@web.de>,
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

#include "findfilemodel.h"
#include <QFileIconProvider>
#include <QDirIterator>
#include <QTextStream>
#include <QUrl>
#include <QTextDocumentFragment>
#include <QTimer>
#include <QApplication>
#include <QTextDocument>

FindFileModel::FindFileModel(QObject *parent) :
    QStandardItemModel(parent)
{
    connect(&futureWatcher,SIGNAL(finished()),this,SLOT(findInFilesFinished()));
    connect(&futureWatcher, SIGNAL(canceled()), this, SLOT(restoreOverrideCursor()));
}

QString FindFileModel::fileName(const QModelIndex &index) const
{
    return QFileInfo(itemFromIndex(index)->data(Qt::UserRole + 1).toString()).fileName();
}

QString FindFileModel::filePath(const QModelIndex &index) const
{
    return QFileInfo(itemFromIndex(index)->data(Qt::UserRole + 1).toString()).filePath();
}

bool FindFileModel::remove(const QModelIndex &index)
{
    QStandardItem * item = itemFromIndex(index);
    if(!item)
    {
        qWarning("FindFileModel::remove failed: itemFromIndex returned NULL");
        return false;
    }
    QString filePath = item->data(Qt::UserRole + 1).toString();
    bool b = QFile::remove(filePath);
    if(b)
         this->removeRow(index.row(),index.parent());
    return b;
}

QFileInfo FindFileModel::fileInfo(const QModelIndex &index) const
{
    return QFileInfo(itemFromIndex(index)->data(Qt::UserRole + 1).toString());
}

void FindFileModel::appendFile(QString filePath)
{
    QFileInfo info(filePath);
    if(info.path().isEmpty() || info.path() == ".")
    {
        qWarning("FindFileModel::appendFile failed: filePath must contain the full path including the file name");
        return;
    }

    QString filePathTrunc = info.filePath();

    while(filePathTrunc.count(QDir::separator()) > 1)
      filePathTrunc.remove(0,filePathTrunc.indexOf(QDir::separator())+1);

    QStandardItem * fileItem = new QStandardItem(filePathTrunc);
    fileItem->setIcon(QFileIconProvider().icon(info));
    fileItem->setData(filePath,Qt::UserRole + 1); // store as user data
    appendRow(fileItem);
}

bool FindFileModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
     QStandardItemModel::setData(index,value,role);

     //value can be folder/filename or simply filename
     return QFile::rename(filePath(index),fileInfo(index).path() + QDir::separator() + QFileInfo(value.toString()).fileName());
}

QStringList FindFileModel::mimeTypes() const
{
    return QStringList(QString("text/uri-list"));
}

QMimeData *FindFileModel::mimeData(const QModelIndexList &indexes) const
{
    QList<QUrl> urls;
    for(QModelIndexList::ConstIterator it = indexes.constBegin(); it != indexes.constEnd(); ++it)
    {
        urls+=QUrl::fromLocalFile(this->filePath(*it));
    }
    QMimeData * mimeData = new QMimeData();
    mimeData->setUrls(urls);
    return mimeData;
}

 // this method may be called multiple times if the user is typing a search word
void FindFileModel::findInFiles(const QString& fileName, const QString &content,const QString &path)
{

    if(path.isEmpty() || (fileName.isEmpty() && content.isEmpty()))
        return;

    QStringList files;
    QDirIterator it(path, QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        QString filePath = it.next();
        if(it.fileInfo().isFile())
            files << filePath;
    }

    if(future.isRunning())
        future.cancel();
    else
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    fileContainsFunctor.content = content;
    fileContainsFunctor.fileName = fileName;

    future = QtConcurrent::filtered(files,fileContainsFunctor);

    futureWatcher.setFuture(future);

    // sometimes, wait cursor persists, this is a workaround
    QTimer::singleShot(5000,this,SLOT(restoreOverrideCursor()));
}

void FindFileModel::findInFilesFinished()
{
    foreach(QString fileName, future.results())
        this->appendFile(fileName);
    QApplication::restoreOverrideCursor();
}

void FindFileModel::restoreOverrideCursor()
{
    QApplication::restoreOverrideCursor();
}

bool FindFileModel::FileContains::operator ()(const QString& htmlFilePath)
{
        if(!fileName.isEmpty() && !content.isEmpty())
           return QFileInfo(htmlFilePath).baseName().contains(fileName, Qt::CaseInsensitive) || fileContentContains(htmlFilePath);
        else if(!content.isEmpty())
            return fileContentContains(htmlFilePath);
        else
            return QFileInfo(htmlFilePath).baseName().contains(fileName, Qt::CaseInsensitive);
}

bool FindFileModel::FileContains::fileContentContains(const QString &htmlFilePath)
{
    QFile file(htmlFilePath);
    if(file.open(QIODevice::ReadOnly))
    {
      QTextStream in(&file);
      //QTextDocumentFragment doc = QTextDocumentFragment::fromHtml(in.readAll());
      //QString noteText = doc.toPlainText();
      //return noteText.contains(content, Qt::CaseInsensitive);

      // remove this string here exactly once
      const static QString whiteSpacePreWrap = "p, li { white-space: pre-wrap; }";
      QString text = in.readAll();
      int index;
      if((index = text.indexOf(whiteSpacePreWrap)) != -1)
      {
          text.remove(index,whiteSpacePreWrap.size());
      }

#if QT_VERSION >= 0x050000
      return text.remove(QRegExp("<[^>]*>")).contains(content.toHtmlEscaped(),Qt::CaseInsensitive);
#else
      return text.remove(QRegExp("<[^>]*>")).contains(Qt::escape(content),Qt::CaseInsensitive);
#endif
    }
    return false;
}
