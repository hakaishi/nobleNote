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
#include "treemodel.h"
#include "htmlnotereader.h"
#include <QFile>
#include <QDirIterator>
#include <QMessageBox>
#include <QSettings>
#include <QtConcurrentMap>

Backup::Backup(QWidget *parent): QDialog(parent){
     setupUi(this);

     splitter = new QSplitter(groupBox);
     gridLayout_2->addWidget(splitter);
     treeView = new QTreeView(splitter);
     frame = new QFrame(splitter);
     gridLayout3 = new QGridLayout(frame);
     label = new QLabel(frame);
     label->setText(tr("Preview of the selected backup"));
     gridLayout3->addWidget(label, 0, 0, 1, 1);
     textEdit = new QTextEdit(this);
     textEdit->setDisabled(frame);
     gridLayout3->addWidget(textEdit, 1, 0, 1, 1);

     QStringList headers;
     headers << tr("Backups") << tr("Date");

     QString path = QSettings().value("backupDirPath").toString();
     QStringList files;
     QDirIterator it(path, QDirIterator::Subdirectories);
     while(it.hasNext())
     {
         QString filePath = it.next();
         if(it.fileInfo().isFile())
            files << filePath;
     }

     textDocument = new QTextDocument(this);
     textEdit->setDocument(textDocument);

     model = new TreeModel(headers, files, textDocument);
     treeView->setModel(model);

     for(int column = 0; column < model->columnCount(); ++column)
        treeView->resizeColumnToContents(column);

     treeView->setAlternatingRowColors(true);
     treeView->setSelectionBehavior(QAbstractItemView::SelectItems);
     treeView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
     treeView->setSortingEnabled(true);

     deleteOldButton = new QPushButton(tr("&delete all old backups and file entries"),this);
     buttonBox->addButton(deleteOldButton ,QDialogButtonBox::ActionRole);

     //TODO: should be selectionChanged instead of activated...
     connect(treeView, SIGNAL(activated(QModelIndex)), this, SLOT(showPreview(QModelIndex)));
     connect(this, SIGNAL(handleBackupsSignal()), this, SLOT(handleBackups()));
     connect(this, SIGNAL(accepted()), this, SLOT(restoreBackup()));
     connect(deleteOldButton, SIGNAL(clicked(bool)), this, SLOT(deleteOldBackupsAndFileEntries()));
}

void Backup::showPreview(const QModelIndex &idx)
{
     QString str = idx.data().toString();
     QString text;
     foreach(QString s, model->contents)
       if(s.contains(str + "\t"))
         text = s.remove(str + "\t");
     textEdit->setText(text);
}

void Backup::restoreBackup()
{
     QString fileName = treeView->selectionModel()->currentIndex().data().toString();
     if(!QFile(QSettings().value("backupDirPath").toString() + fileName).exists())
       return;
     else
     {
        ;//TODO:Remove or Override Note
     }
}

void Backup::handleBackups()
{
//TODO: Create and handle Backups here
}

void Backup::getUuidList(){
     QFutureIterator<QUuid> it(future1->future());
     while(it.hasNext())
       notesUuids << it.next();
}

void Backup::deleteOldBackupsAndFileEntries(){
     notesUuids.clear(); //make sure that it's empty

     //searching for existing Notes
     QDirIterator itFiles(QSettings().value("noteDirPath").toString(),
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

     QObject::connect(future1, SIGNAL(finished()), this, SLOT(getUuidList()));
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
     QDirIterator itBackup(QSettings().value("backupDirPath").toString(),
                                              QDirIterator::Subdirectories);
     QStringList backups;
     while(itBackup.hasNext()){
       QString filePath = itBackup.next();
       if(itBackup.fileInfo().isFile())
         backups << filePath;
     }

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

     if(QMessageBox::warning(this,tr("Deleting backups and file entries"),
         tr("Do you really want to delete the backups and entries for the "
            "following files?\n\n%1\nYou won't be able to restore them!").arg(
         redundantBackupList),
         QMessageBox::Yes | QMessageBox::Abort) != QMessageBox::Yes)
       return;

     progressDialog = new QProgressDialog(this);
     progressDialog->setLabelText(QString(tr("Progressing files...")));

     future2 = new QFutureWatcher<void>(this);
     future2->setFuture(QtConcurrent::map(backupsAndUuids, actualRemoval));

     QObject::connect(future2, SIGNAL(finished()), progressDialog, SLOT(reset()));
     QObject::connect(progressDialog, SIGNAL(canceled()), future2, SLOT(cancel()));
     QObject::connect(future2, SIGNAL(progressRangeChanged(int,int)), progressDialog,
                                                                SLOT(setRange(int,int)));
     QObject::connect(future2, SIGNAL(progressValueChanged(int)), progressDialog,
                                                                SLOT(setValue(int)));

     progressDialog->exec();
}
