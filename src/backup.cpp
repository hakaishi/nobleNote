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
#include "trash.h"
#include <QDirIterator>
#include <QMessageBox>
#include <QSettings>
#include <QtConcurrentMap>
#include <QAbstractItemModel>

Backup::Backup(QWidget *parent)
{
     parent_ = parent;
     getNotes(); //Searches for notes and backups. For the backups with no notes it will create the trees children.
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
     progressDialog1 = new QProgressDialog(parent_);
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
          showTrash();
          return;
     }

     progressReceiver2 = new ProgressReceiver(this);
     progressDialog2 = new QProgressDialog(parent_);
     progressDialog2->setLabelText(QString(tr("Indexing trash...")));
     setupBackup.p = progressReceiver2;
     setupBackup.hash = &backupDataHash;
     future2 = new QFutureWatcher<void>(this);
     future2->setFuture(QtConcurrent::map(backupFiles, setupBackup));

     QObject::connect(progressReceiver2,SIGNAL(valueChanged(int)),progressDialog2, SLOT(setValue(int)));
     QObject::connect(future2, SIGNAL(finished()), this, SLOT(showTrash()));
     QObject::connect(future2, SIGNAL(finished()), progressDialog2, SLOT(reset()));
     QObject::connect(progressDialog2, SIGNAL(canceled()), future2, SLOT(cancel()));

     progressDialog2->show();
}

void Backup::showTrash()
{
     trash = new Trash(&backupDataHash, parent_);
     connect(trash, SIGNAL(destroyed()), this, SLOT(deleteLater()));
     trash->show();
}
