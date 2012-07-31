#include "findfilemodel.h"
#include <QFileIconProvider>
#include <QDirIterator>
#include <QTextStream>
#include <QUrl>
#include <QTextDocumentFragment>
#include <QtDebug>
#include <QApplication>


FindFileModel::FindFileModel(QObject *parent) :
    QStandardItemModel(parent)
{
    connect(&futureWatcher,SIGNAL(finished()),this,SLOT(findInFilesFinished()));
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
    bool b = QFile::remove(itemFromIndex(index)->data(Qt::UserRole + 1).toString());
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

    while(filePathTrunc.count("/") > 1)
      filePathTrunc.remove(0,filePathTrunc.indexOf("/")+1);

    QStandardItem * fileItem = new QStandardItem(filePathTrunc);
    fileItem->setIcon(QFileIconProvider().icon(info));
    fileItem->setData(filePath,Qt::UserRole + 1); // store as user data
    appendRow(fileItem);
}

bool FindFileModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
     QStandardItemModel::setData(index,value,role);
     return QFile::rename(filePath(index),fileInfo(index).path() + "/" + value.toString());
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

}

void FindFileModel::findInFilesFinished()
{
    foreach(QString fileName, future.results())
        this->appendFile(fileName);
    QApplication::restoreOverrideCursor();
}

bool FindFileModel::FileContains::operator ()(const QString& htmlFilePath)
{
        if(!fileName.isEmpty() && !content.isEmpty())
           return QFileInfo(htmlFilePath).baseName().contains(fileName, Qt::CaseInsensitive) && fileContentContains(htmlFilePath);
        else if(!content.isEmpty())
            return fileContentContains(htmlFilePath);
        else
            return QFileInfo(htmlFilePath).baseName().contains(fileName, Qt::CaseInsensitive);
}

bool FindFileModel::FileContains::fileContentContains(const QString &htmlFilePath)
{
    QFile file(htmlFilePath);
    if(file.open(QIODevice::ReadOnly)){
      QTextStream in(&file);
      QTextDocumentFragment doc = QTextDocumentFragment::fromHtml(in.readAll());
      QString noteText = doc.toPlainText();
      return noteText.contains(content, Qt::CaseInsensitive);
    }
    return false;
}

