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
     treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

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

     connect(treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(showPreview()));
     connect(deleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteBackup()));
     connect(restoreButton, SIGNAL(clicked(bool)), this, SLOT(restoreBackup()));
}

void Backup::setupTreeData()
{
     treeWidget->clear(); //if there already is any data

     //get backup uuids
     QDir backupDir(QSettings().value("backup_dir_path").toString());
     QList<QFileInfo> backupList = backupDir.entryInfoList(QDir::Files, QDir::Name);
     QStringList backupUuids;
     foreach(QFileInfo backup, backupList)
          backupUuids << "{" + backup.fileName() + "}";

     //get note files
     QDirIterator itFiles(QSettings().value("root_path").toString(),
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
     {
          textEdit->clear();
          return;
     }

     QHash<QString,QStringList> backupHash;
     foreach(QString str, backupUuids)
     {
          str.remove("{");
          str.remove("}");
          QStringList data = getFileData(QSettings().value("backup_dir_path").toString()
                                   + "/" + str);
          QString uuid = data.takeFirst();
          backupHash.insert(uuid, data);
     }

     foreach(QString key, backupHash.keys())
     {
          QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);
          item->setText(0,backupHash[key].first());
          item->setData(0,Qt::UserRole,backupHash[key]);
     }

     textEdit->clear();
}

QStringList Backup::getFileData(const QString &file)
{
     AbstractNoteReader *reader = new HtmlNoteReader(file,document);
     QStringList list;
     QString uuid = reader->uuid();
     list << uuid << reader->title() << QFileInfo(file).absoluteFilePath() << document->toHtml();
     return list;
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
          dataList.takeFirst(); //removing title
          files << dataList.first();
     }

     QString backupsToBeDeleted;
     foreach(QString str, files)
         backupsToBeDeleted += (str+"\n");

     if(QMessageBox::warning(this,tr("Deleting backups and file entries"),
          tr("Do you really want to delete the backups and entries for the "
             "following files?\n\n%1\nYou won't be able to restore them!").arg(
          backupsToBeDeleted),QMessageBox::Yes | QMessageBox::Abort) != QMessageBox::Yes)
        return;

     foreach(QString file, files)
     {
          if(QFile(file).exists())
            QFile(file).remove();

          QString uuid = file;
          uuid.remove(QSettings().value("backup_dir_path").toString() + "/");
          QSettings().remove("Notes/{" + uuid + "}_size");
          QSettings().remove("Notes/{" + uuid + "}_cursor_position");
     }

     foreach(QTreeWidgetItem *item, itemList)
          delete item;
}
