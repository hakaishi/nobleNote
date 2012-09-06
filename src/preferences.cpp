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

#include "preferences.h"
#include "htmlnotereader.h"
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <QtConcurrentMap>
#include <QDirIterator>

Preferences::Preferences(QWidget *parent): QDialog(parent){
     setupUi(this);

     settings = new QSettings(this);

     dontQuit->setChecked(settings->value("Dont_quit_on_close",false).toBool());
     convertNotes->setChecked(settings->value("convert_notes",true).toBool());
     sizeSpinHeight->setValue(settings->value("Standard_height_for_all_notes",250).toInt());
     sizeSpinWidth->setValue(settings->value("Standard_width_for_all_notes",335).toInt());

     connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
     connect(browseButton, SIGNAL(clicked(bool)), this, SLOT(openDir()));
     connect(pushButton, SIGNAL(clicked(bool)), this, SLOT(deleteOldBackupsAndFileEntries()));
}

Preferences::~Preferences(){}

void Preferences::showEvent(QShowEvent* show_pref){
     pathLabel->setText(settings->value("rootPath").toString());
     rootPath = settings->value("rootPath").toString();
     originalRootPath = rootPath;

     QWidget::showEvent(show_pref);
}

void Preferences::saveSettings(){
     if(!settings->isWritable()){
       QMessageBox::warning(this,tr("Warning"),tr("Could not write settings!"));
     }

     if(rootPath != originalRootPath){
       settings->setValue("rootPath",rootPath);
       settings->setValue("noteDirPath",rootPath + "/notes");
       settings->setValue("backupDirPath",rootPath + "/backups");
       sendPathChanged();
     }

     settings->setValue("Dont_quit_on_close", dontQuit->isChecked());
     settings->setValue("convert_notes", convertNotes->isChecked());
     settings->setValue("Standard_height_for_all_notes", sizeSpinHeight->value());
     settings->setValue("Standard_width_for_all_notes", sizeSpinWidth->value());

     accept();
}

void Preferences::openDir(){
     QString path;
     path = QFileDialog::getExistingDirectory(this,
                  tr("Open Directory"), rootPath, QFileDialog::ShowDirsOnly
                  | QFileDialog::DontResolveSymlinks);
     QFileInfo file(path);
     if(!file.isWritable() && !path.isEmpty()){
       QMessageBox msgBox;
       msgBox.setWindowTitle(tr("Warning"));
       msgBox.setIcon(QMessageBox::Critical);
       msgBox.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window);
       QTimer::singleShot(6000, &msgBox, SLOT(close()));
       msgBox.setInformativeText(tr("The path \"%1\" is not writable!").arg(file.filePath()));
       msgBox.exec();
       return;
     }
     if(!path.isEmpty()){
       rootPath = path;
       pathLabel->setText(rootPath);
     }

}

void Preferences::getUuidList(){
     QFutureIterator<QUuid> it(future1->future());
     while(it.hasNext())
       notesUuids << it.next();
}

void Preferences::deleteOldBackupsAndFileEntries(){
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

void Preferences::progressChanges(){
     //get backup files
     QDirIterator itBackup(settings->value("backupDirPath").toString(),
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
