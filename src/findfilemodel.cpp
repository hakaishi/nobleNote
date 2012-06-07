#include "findfilemodel.h"
#include <QFileIconProvider>
#include <QDirIterator>
#include <QTextStream>
#include <QUrl>
#include <QTextDocumentFragment>
#include <QtDebug>


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


/*static*/ QStringList  FindFileModel::find0(const QString &searchName, const QString &searchText,const QString& path)
{
    if(searchName.isEmpty() && searchText.isEmpty())
        return QStringList();

    QStringList files;
    QStringList results; // found files

    QDirIterator it(path, QDirIterator::Subdirectories);
    while(it.hasNext())
      files << it.next();

    if(!searchText.isEmpty())
      files = findinFiles0(files, searchText);

    while(!files.isEmpty()){
      QString file = files.takeFirst();
      QFileInfo info(file);

      // if no fileName given, search for content only, if filenName given, search for files that contain that name
      if(searchName.isEmpty() || (!searchName.isEmpty() && !info.isDir() && info.baseName().contains(searchName,Qt::CaseInsensitive)))
        results.append(file);
    }
    return results;
}

void FindFileModel::find(const QString &text, const QString &path)
{
    if(text.isEmpty() || path.isEmpty())
        return;

    QStringList files;
    QDirIterator it(path, QDirIterator::Subdirectories);
    while(it.hasNext())
      files << it.next();

//    QList<QString> results = blockingFindInFiles(files,text);

//    foreach(QString str,results)
//        this->appendFile(str);
    //findInFiles(files,text);

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

/*static*/ QStringList FindFileModel::findinFiles0(const QStringList &files, const QString &text/*, const QString &path*/){
//     QProgressDialog progressDialog(this);
//     progressDialog.setCancelButtonText(tr("&Cancel"));
//     progressDialog.setRange(0, files.size());
//     progressDialog.setWindowTitle(tr("Find Files"));

     QStringList foundFiles;

     for(int i = 0; i < files.size(); ++i){
//        progressDialog.setValue(i);
//        progressDialog.setLabelText(tr("Searching file number %1 of %2...")
//          .arg(i).arg(files.size()));
//        qApp->processEvents();

//        if(progressDialog.wasCanceled())
//          break;

        //QFile file(QDir(path).absoluteFilePath(files[i]));
         QFile file(files[i]);

        if(file.open(QIODevice::ReadOnly)){
          QTextStream in(&file);
          QTextDocumentFragment doc = QTextDocumentFragment::fromHtml(in.readAll());
          QString noteText = doc.toPlainText();
          if(noteText.contains(text))
            foundFiles << files[i];
        }
        if(file.isOpen())
          file.close();
     }
     return foundFiles;
}

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

    fileContainsFunctor.text = content;
    fileContainsFunctor.content = content;
    fileContainsFunctor.fileName = fileName;

//    if(!fileName.isEmpty() && !content.isEmpty())
//        fileContainsFunctor.Search = FileContains::NameAndContent;
//    else if(!content.isEmpty() && fileName.isEmpty())
//        fileContainsFunctor.Search = FileContains::Content;
//    else if( ! fileName.isEmpty() && content.isEmpty())
//        fileContainsFunctor.Search = FileContains::Name;

    future = QtConcurrent::filtered(files,fileContainsFunctor);

    futureWatcher.setFuture(future);
}

QList<QString> FindFileModel::blockingFindInFiles(const QStringList &files, const QString &text)
{
    fileContainsFunctor.text = text;
    fileContainsFunctor.Search = FileContains::Content;
    return QtConcurrent::blockingFiltered(files,fileContainsFunctor);
}

void FindFileModel::findInFilesFinished()
{
    foreach(QString fileName, future.results())
        this->appendFile(fileName);
}


bool FindFileModel::FileContains::operator ()(const QString& htmlFilePath)
{
//    qDebug() << Search;
//    switch(Search)
//    {
//    case Name:
//        return QFileInfo(htmlFilePath).baseName().contains(text);
//        break;
//    case Content:
//        return fileContentContains(htmlFilePath);
//        break;
//    case NameAndContent:
//        return QFileInfo(htmlFilePath).baseName().contains(text) && fileContentContains(htmlFilePath);
//        break;
//    default:;
//    }
//    return false;
        if(!fileName.isEmpty() && !content.isEmpty())
           return QFileInfo(htmlFilePath).baseName().contains(fileName) && fileContentContains(htmlFilePath);
        else if(!content.isEmpty())
            return fileContentContains(htmlFilePath);
        else
            return QFileInfo(htmlFilePath).baseName().contains(fileName);
}

bool FindFileModel::FileContains::fileContentContains(const QString &htmlFilePath)
{
    QFile file(htmlFilePath);
    if(file.open(QIODevice::ReadOnly)){
      QTextStream in(&file);
      QTextDocumentFragment doc = QTextDocumentFragment::fromHtml(in.readAll());
      QString noteText = doc.toPlainText();
      return noteText.contains(content);
    }
    return false;
}

