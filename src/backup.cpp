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
#include <QDirIterator>
#include <QMessageBox>
#include <QSettings>
#include <QtConcurrentMap>
#include <QAbstractItemModel>

Backup::Backup(QWidget *parent): QDialog(parent){
     setupUi(this);

     setAttribute(Qt::WA_DeleteOnClose);

     treeWidget->sortByColumn(0,Qt::AscendingOrder);
     // TODO flickcharm here

     getNotes(); //Searches for notes and backups. For the backups with no notes it will create the trees children.

     connect(treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(showPreview()));
     connect(deleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteBackup()));
     connect(restoreButton, SIGNAL(clicked(bool)), this, SLOT(restoreBackup()));
}

void Backup::closeEvent(QCloseEvent* e)
{
     if(future1->isCanceled() && future2->isCanceled())
     {
        future1->waitForFinished();
        future2->waitForFinished();
     }
     delete backupDataHash;

     QDialog::closeEvent(e);
}

void Backup::getNoteUuidList()
{
     QFutureIterator<QString> it(future1->future());
     while(it.hasNext())
       noteUuidList << it.next();
}

void Backup::getNotes()
{
     //get note files
     QDirIterator itFiles(QSettings().value("root_path").toString(),
                                              QDirIterator::Subdirectories);
     while(itFiles.hasNext()){
       QString filePath = itFiles.next();
       if(itFiles.fileInfo().isFile())
         noteFiles << filePath;
     }

     progressReceiver1 = new ProgressReceiver(this);
     progressDialog1 = new QProgressDialog(this);
     progressDialog1->setLabelText(QString(tr("Indexing notes...")));
     getUuid.p = progressReceiver1;
     future1 = new QFutureWatcher<QString>(this);
     future1->setFuture(QtConcurrent::mapped(noteFiles, getUuid));

     QObject::connect(progressReceiver1,SIGNAL(valueChanged(int)),progressDialog1, SLOT(setValue(int)));
     QObject::connect(future1, SIGNAL(finished()), this, SLOT(getNoteUuidList()));
     QObject::connect(future1, SIGNAL(finished()), this, SLOT(setupBackups()));
     QObject::connect(future1, SIGNAL(finished()), progressDialog1, SLOT(reset()));
     QObject::connect(progressDialog1, SIGNAL(canceled()), future1, SLOT(cancel()));

     progressDialog1->show();
}

void Backup::setupBackups()
{
     backupFiles.clear(); //remove old files

     //get backup uuids
     QDir backupDir(QSettings().value("backup_dir_path").toString());
     backupFiles = backupDir.entryInfoList(QDir::Files, QDir::Name);

     foreach(QString uuid, noteUuidList)
     {
          if(backupFiles.contains(QFileInfo(QSettings().value("backup_dir_path").toString()
                                  + "/" + uuid.mid(1,36))))
          {
             backupFiles.removeOne(QFileInfo(QSettings().value("backup_dir_path").toString()
                                   + "/" + uuid.mid(1,36)));
             backupFiles.removeOne(QFileInfo(QSettings().value("backup_dir_path").toString()
                                   + "/" + uuid));
          }
     }

     if(backupFiles.isEmpty())
     {
          textEdit->clear();
          return;
     }

     progressReceiver2 = new ProgressReceiver(this);
     progressDialog2 = new QProgressDialog(this);
     progressDialog2->setLabelText(QString(tr("Indexing trash...")));
     setupBackup.p = progressReceiver2;
     backupDataHash = new QHash<QString,QStringList>;
     setupBackup.hash = backupDataHash;
     future2 = new QFutureWatcher<void>(this);
     future2->setFuture(QtConcurrent::map(backupFiles, setupBackup));

     QObject::connect(progressReceiver2,SIGNAL(valueChanged(int)),progressDialog2, SLOT(setValue(int)));
     QObject::connect(future2, SIGNAL(finished()), this, SLOT(setupChildren()));
     QObject::connect(future2, SIGNAL(finished()), progressDialog2, SLOT(reset()));
     QObject::connect(progressDialog2, SIGNAL(canceled()), future2, SLOT(cancel()));

     progressDialog2->show();
}

void Backup::setupChildren()
{
     QHash<QString,QStringList> hash = *backupDataHash;
     foreach(QString key, hash.keys())
     {
          QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);
          item->setText(0,hash[key].first()); //title
          item->setData(0,Qt::UserRole,hash[key]); //title, path and content
     }
}

void Backup::showPreview()
{
     if(treeWidget->currentItem() == NULL) //Prevents program crush
     {
          textEdit->clear();
          return;
     }

     if(!treeWidget->currentItem()->isSelected())
     {
          if(treeWidget->selectedItems().count() != 1)
             textEdit->clear();
          else
             textEdit->setText(treeWidget->selectedItems().first()->data(0,Qt::UserRole).toStringList().last());
     }
     else
        textEdit->setText(treeWidget->currentItem()->data(0,Qt::UserRole).toStringList().last());
}

void Backup::restoreBackup()
{
     if(treeWidget->selectedItems().isEmpty())
       return;
     foreach(QTreeWidgetItem *item, treeWidget->selectedItems())
     {
          QStringList dataList = item->data(0,Qt::UserRole).toStringList();
          QString title = dataList.takeFirst();
          if(!QFile(dataList.first()).exists())
            return;
          else
          {
             if(!QDir(QSettings().value("root_path").toString()+"/restored notes").exists())
               QDir().mkpath(QSettings().value("root_path").toString()+"/restored notes");
             QFile(dataList.first()).copy(QSettings().value("root_path").toString()+"/restored notes/"+title);
          }
          delete item;
     }
}

void Backup::deleteBackup()
{
     if(treeWidget->selectedItems().isEmpty())
        return;

     QStringList files;
     QList<QTreeWidgetItem*> itemList = treeWidget->selectedItems();
     foreach(QTreeWidgetItem *item, itemList)
     {
          QStringList dataList = item->data(0,Qt::UserRole).toStringList();
          dataList.takeFirst(); //removing title from the list
          files << dataList.first();
     }

     QString backupsToBeDeleted;
     foreach(QString str, files)
         backupsToBeDeleted += (str+"\n");

     if(QMessageBox::warning(this,tr("Delete multiple notes"),
          tr("Are you sure you want to permanently delete these notes?\n\n%1").arg(
          QDir::toNativeSeparators(backupsToBeDeleted)),QMessageBox::Yes | QMessageBox::Abort) != QMessageBox::Yes)
        return;

     foreach(QString file, files)
     {
          if(QFile(file).exists())
            QFile(file).remove();

          QString uuid = file;
          uuid.remove(QSettings().value("backup_dir_path").toString() + "/");
          QSettings().remove("Notes/" + QUuid(uuid).toString() + "_size");
          QSettings().remove("Notes/" + QUuid(uuid).toString() + "_cursor_position");
     }

     foreach(QTreeWidgetItem *item, itemList)
          delete item;
}
