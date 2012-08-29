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
#include <QProgressDialog>
#include <QFutureWatcher>
#include <QtConcurrentFilter>
#include <QDirIterator>

Preferences::Preferences(QWidget *parent): QDialog(parent){
     setupUi(this);

     settings = new QSettings(this);

     dontQuit->setChecked(settings->value("Dont_quit_on_close",false).toBool());
     convertNotes->setChecked(settings->value("convert_notes",true).toBool());
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
       QMessageBox msgBox;
       msgBox.setWindowTitle(tr("Warning"));
       msgBox.setIcon(QMessageBox::Critical);
       msgBox.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window);
       QTimer::singleShot(6000, &msgBox, SLOT(close()));
       msgBox.setInformativeText(tr("nobleNote.conf is not writable!"));
       msgBox.exec();
     }

     if(rootPath != originalRootPath){
       settings->setValue("rootPath",rootPath);
       settings->setValue("noteDirPath",rootPath + "/notes");
       settings->setValue("backupDirPath",rootPath + "/backups");
       sendPathChanged();
     }

     settings->setValue("Dont_quit_on_close", dontQuit->isChecked());
     settings->setValue("convert_notes", convertNotes->isChecked());

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

void Preferences::deleteOldBackupsAndFileEntries(){
     if(QMessageBox::warning(this,tr("Deleting backups and file entries"),
           tr("Do you really want to delete the backups and entries? You "
              "won't be able to restore them!"),
           QMessageBox::Yes | QMessageBox::Abort) != QMessageBox::Yes)
        return;

     //get notes
     QStringList notes;
     QDirIterator itFiles(QSettings().value("noteDirPath").toString(), QDirIterator::Subdirectories);
     while(itFiles.hasNext()){
        QString filePath = itFiles.next();
        if(itFiles.fileInfo().isFile())
            notes << filePath;
     }
     getFilesFunctor.notes = notes;

     //get backup files
     QStringList backups;
     QDirIterator itBackup(settings->value("backupDirPath").toString(), QDirIterator::Subdirectories);
     while(itBackup.hasNext()){
        QString filePath = itBackup.next();
        if(itBackup.fileInfo().isFile())
            backups << filePath;
     }

     //get QSettings Uuids and backups
     QStringList backupsAndUuids = backups + QSettings().allKeys().filter("Notes/");

     QProgressDialog dialog;
     dialog.setLabelText(QString(tr("Progressing files...")));

     // Create a QFutureWatcher and connect signals and slots.
     QFutureWatcher<QString> futureWatcher;
     QObject::connect(&futureWatcher, SIGNAL(finished()), &dialog, SLOT(reset()));
     QObject::connect(&dialog, SIGNAL(canceled()), &futureWatcher, SLOT(cancel()));
     QObject::connect(&futureWatcher, SIGNAL(progressRangeChanged(int,int)), &dialog, SLOT(setRange(int,int)));
     QObject::connect(&futureWatcher, SIGNAL(progressValueChanged(int)), &dialog, SLOT(setValue(int)));

     // Start the computation.
     futureWatcher.setFuture(QtConcurrent::filtered(backupsAndUuids, getFilesFunctor));

     dialog.exec();

     futureWatcher.waitForFinished();
}

bool Preferences::getFiles::operator()(const QString& backupAndUuid){
     if(removeBackup(backupAndUuid) ^ removeSettingsUuid(backupAndUuid))
       return true;
     else
       return false;
}

bool Preferences::getFiles::removeBackup(const QString& backupAndUuid){
     bool a = false;
     if(!backupAndUuid.contains("Notes/")){
       foreach(QString file, notes)
         if(!(HtmlNoteReader::uuid(file) == HtmlNoteReader::uuid(backupAndUuid)))
           if(QFile::remove(backupAndUuid))
             a = true;
     }
     return a;
}

bool Preferences::getFiles::removeSettingsUuid(const QString& backupAndUuid){
     bool a = false;
     if(backupAndUuid.contains("Notes/")){
       foreach(QString uuid, notes){
         QString uuidSize = "Notes/"+HtmlNoteReader::uuid(uuid)+"_size";
         QString uuidCursorPosition = "Notes/"+HtmlNoteReader::uuid(uuid)+"_cursor_position";
         if((backupAndUuid != uuidSize) && (backupAndUuid != uuidCursorPosition)){
           QSettings().remove(backupAndUuid);
           a = true;
         }
       }
     }
     return a;
}

/*bool Preferences::processFiles(QStringList files){
     //get uuid form files
     QStringList uuidList;
     foreach(QString uuid, files)
       uuidList << HtmlNoteReader::uuid(uuid);

     //get backup files that are to be removed
     QStringList uuidToBeRemoved;
     foreach(QString uuidFileName, backups)
       if(!uuidList.contains("{"+QFileInfo(uuidFileName).fileName()+"}"))
         uuidToBeRemoved << uuidFileName;

     //create a QString of backups that are to be removed for a message box
     QString listOfUuidToBeRemoved;
     foreach(QString str, uuidToBeRemoved)
       listOfUuidToBeRemoved += QFileInfo(str).fileName()+"\n";
     if(uuidToBeRemoved.isEmpty()){
       QMessageBox::information(this,tr("Clean"), tr("No redundant backups present."));
       return;
     }
     if(QMessageBox::warning(this,tr("Deleting backups and file entries"),
           tr("Do you really want to delete the backups and entries for the "
              "following files?\n\n%1\nYou won't be able to restore them!").arg(
           listOfUuidToBeRemoved),
           QMessageBox::Yes | QMessageBox::Abort) != QMessageBox::Yes)
        return;

     //removing entries/backups
     QString removeUuid;
     foreach(removeUuid, uuidToBeRemoved){
         QFile::remove(removeUuid);
         QSettings().remove("{"+QFileInfo(removeUuid).fileName()+"}_size");
         QSettings().remove("{"+QFileInfo(removeUuid).fileName()+"}_cursor_position");
     }

}*/
