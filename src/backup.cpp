/* nobleNote, a note taking application
 * Copyright (C) 2012 Christian Metscher <hakaishi@web.de>,
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

#include "backup.h"
#include "htmlnotereader.h"
#include "abstractnotereader.h"
#include <QDirIterator>
#include <QMessageBox>
#include <QSettings>
#include <QtConcurrentMap>
#include <QAbstractItemModel>

Backup::Backup(QWidget *parent): QDialog(parent){
     setupUi(this);

     splitter = new QSplitter(groupBox);
     gridLayout_2->addWidget(splitter);
     treeWidget = new QTreeWidget(splitter);
     treeWidget->setAlternatingRowColors(true);
     treeWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
     treeWidget->setSortingEnabled(true);
     treeWidget->setHeaderLabel(tr("Backups of deleted notes"));
     frame = new QFrame(splitter);
     gridLayout3 = new QGridLayout(frame);
     label = new QLabel(frame);
     label->setText(tr("Preview of the selected backup"));
     gridLayout3->addWidget(label, 0, 0, 1, 1);
     textEdit = new QTextEdit(this);
     textEdit->setDisabled(frame);
     gridLayout3->addWidget(textEdit, 1, 0, 1, 1);

     document = new QTextDocument(this);
     textEdit->setDocument(document);

     setupTreeData();

     deleteOldButton = new QPushButton(tr("&Delete all old backups and file entries"),this);
     buttonBox->addButton(deleteOldButton ,QDialogButtonBox::ActionRole);

     //TODO: should be selectionChanged instead of activated...
     connect(treeWidget, SIGNAL(activated(QModelIndex)), this, SLOT(showPreview(QModelIndex)));
     connect(this, SIGNAL(accepted()), this, SLOT(restoreBackup()));
     connect(deleteOldButton, SIGNAL(clicked(bool)), this, SLOT(deleteOldBackupsAndFileEntries()));
}

void Backup::setupTreeData()
{
     treeWidget->clear(); //if there already is any data

     //get backup uuids
     QDir backupDir(QSettings().value("backupDirPath").toString());
     QList<QFileInfo> backupList = backupDir.entryInfoList(QDir::Files, QDir::Name);
     QStringList backupUuids;
     foreach(QFileInfo backup, backupList)
          backupUuids << "{" + backup.fileName() + "}";

     //get note files
     QDirIterator itFiles(QSettings().value("rootPath").toString(),
                                              QDirIterator::Subdirectories);
     QStringList noteFiles;
     while(itFiles.hasNext()){
       QString filePath = itFiles.next();
       if(itFiles.fileInfo().isFile())
         noteFiles << filePath;
     }
     QStringList noteUuids; //get note uuids
     foreach(QString note, noteFiles)
          noteUuids << getFileData(note).first();

     foreach(QString uuid, noteUuids)
          if(backupUuids.contains(uuid))
             backupUuids.removeOne(uuid);

     if(backupUuids.isEmpty())
          return;

     QHash<QString,QStringList> backupHash;
     foreach(QString str, backupUuids)
     {
          str.remove("{");
          str.remove("}");
          QStringList data = getFileData(QSettings().value("backupDirPath").toString()
                                   + "/" + str);
          QString uuid = data.takeFirst();
          backupHash.insert(uuid, data);
     }

     foreach(QString key, backupHash.keys())
     {
          QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);
          item->setText(0,backupHash[key].takeFirst());
          item->setData(0,Qt::UserRole,backupHash[key]);
     }

     //treeWidget->resizeColumnToContents(0);
}

QStringList Backup::getFileData(const QString &file)
{
     AbstractNoteReader *reader = new HtmlNoteReader(file,document);
     QStringList list;
     QString uuid = reader->uuid();
     list << uuid << reader->title() << QFileInfo(file).absoluteFilePath() << document->toHtml();
     return list;
}

void Backup::showPreview(const QModelIndex &idx)
{
     QStringList data = idx.data(Qt::UserRole).toStringList();
     if(data.isEmpty())
       return;
     textEdit->setText(data.last());
}

void Backup::restoreBackup()
{
     if(!treeWidget->selectionModel()->currentIndex().isValid())
       return;
     if(treeWidget->selectionModel()->currentIndex().data(Qt::UserRole).toStringList().isEmpty())
       return;
     QStringList dataList = treeWidget->selectionModel()->currentIndex().data(Qt::UserRole).toStringList();
     if(!QFile(dataList.first()).exists())
       return;
     else
     {
        qDebug()<<dataList.first();//TODO:restore or overwrite note
     }
}

void Backup::getNoteUuidList(){
     QFutureIterator<QUuid> it(future1->future());
     while(it.hasNext())
       notesUuids << it.next();
}

void Backup::deleteOldBackupsAndFileEntries(){
     notesUuids.clear(); //make sure that it's empty

     //searching for existing Notes
     QDirIterator itFiles(QSettings().value("rootPath").toString(),
                                              QDirIterator::Subdirectories);
     QStringList noteFiles;
     while(itFiles.hasNext()){
       QString filePath = itFiles.next();
       if(itFiles.fileInfo().isFile())
         noteFiles << filePath;
     }

     future1 = new QFutureWatcher<QUuid>(this);

     QUuid (*uuidPtr)(QString) = & HtmlNoteReader::uuid; // function pointer, because uuid method is overloaded
     future1->setFuture(QtConcurrent::mapped(noteFiles, uuidPtr));

     indexDialog = new QProgressDialog(this);
     indexDialog->setLabelText(QString(tr("Indexing notes...")));

     QObject::connect(future1, SIGNAL(finished()), this, SLOT(getNoteUuidList()));
     QObject::connect(future1, SIGNAL(finished()), this, SLOT(progressChanges()));
     QObject::connect(future1, SIGNAL(finished()), indexDialog, SLOT(reset()));
     QObject::connect(indexDialog, SIGNAL(canceled()), future1, SLOT(cancel()));
     QObject::connect(future1, SIGNAL(progressRangeChanged(int,int)),
                indexDialog, SLOT(setRange(int,int)));
     QObject::connect(future1, SIGNAL(progressValueChanged(int)), indexDialog,
                SLOT(setValue(int)));

     indexDialog->exec();
}

void actualRemoval(const QString& backupAndUuid){
     if(!backupAndUuid.contains("Notes/"))
       QFile::remove(backupAndUuid);
     else
       QSettings().remove(backupAndUuid);
}

void Backup::progressChanges(){
     //get backup files
     QStringList backups;
     QDir backupDir(QSettings().value("backupDirPath").toString());
     QList<QFileInfo> backupList = backupDir.entryInfoList(QDir::Files, QDir::Name);
     foreach(QFileInfo backup, backupList)
          backups << backup.absoluteFilePath();

     //add QSettings Uuids to the backups
     QStringList backupsAndUuids = backups + QSettings().allKeys().filter("Notes/");

     //We only need the redundant backups and Uuids
     foreach(QString str, backupsAndUuids){
       if(!str.contains("Notes/") && notesUuids.contains(QFileInfo(str).fileName()))
         backupsAndUuids.removeOne(str);
       if(str.contains("Notes/")){
         QString settings = str;
         settings.remove("Notes/");
         settings.remove("_size");
         settings.remove("_cursor_position");         
         if(notesUuids.contains(settings))
           backupsAndUuids.removeOne(str);
       }
     }

     QString redundantBackupList;
     foreach(QString str, backupsAndUuids)
       if(!str.contains("Notes/"))
         redundantBackupList += (str+"\n");

     if(backupsAndUuids.isEmpty()){
          QMessageBox::information(this, tr("No redundant data!"), tr("no redundant"
                                   " Everything is clean! No redundant data!"));
          return;
     }
     else{
       if(QMessageBox::warning(this,tr("Deleting backups and file entries"),
           tr("Do you really want to delete the backups and entries for the "
              "following files?\n\n%1\nYou won't be able to restore them!").arg(
           redundantBackupList),
           QMessageBox::Yes | QMessageBox::Abort) != QMessageBox::Yes)
         return;
     }

     progressDialog = new QProgressDialog(this);
     progressDialog->setLabelText(QString(tr("Progressing files...")));

     future2 = new QFutureWatcher<void>(this);
     future2->setFuture(QtConcurrent::map(backupsAndUuids, actualRemoval));

     QObject::connect(future2, SIGNAL(finished()), progressDialog, SLOT(reset()));
     QObject::connect(future2, SIGNAL(finished()), this, SLOT(setupTreeData()));
     QObject::connect(progressDialog, SIGNAL(canceled()), future2, SLOT(cancel()));
     QObject::connect(future2, SIGNAL(progressRangeChanged(int,int)), progressDialog,
                                                                SLOT(setRange(int,int)));
     QObject::connect(future2, SIGNAL(progressValueChanged(int)), progressDialog,
                                                                SLOT(setValue(int)));

     progressDialog->exec();
}
