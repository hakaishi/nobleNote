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

#include "preferences.h"
#include "slash.h"
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>

Preferences::Preferences(QWidget *parent): QDialog(parent)
{
     setupUi(this);

     settings = new QSettings(this);

     QFontDatabase db;
     foreach(int size, db.standardSizes())
       fontSizeComboBox->addItem(QString::number(size));

     if(settings->value("isPortable",false).toBool())
          createPortable->setDisabled(true);

     dontQuit->setChecked(settings->value("dont_quit_on_close",false).toBool());
     convertNotes->setChecked(settings->value("convert_notes",true).toBool());
     showSource->setChecked(settings->value("show_source", false).toBool());
     kineticScrolling->setChecked(settings->value("kinetic_scrolling", false).toBool());
     sizeSpinHeight->setValue(settings->value("note_editor_default_size",QSize(335,250)).toSize().height());
     sizeSpinWidth->setValue(settings->value("note_editor_default_size",QSize(335,250)).toSize().width());


     connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
     connect(browseButton, SIGNAL(clicked(bool)), this, SLOT(setNewPaths()));
     connect(kineticScrolling,SIGNAL(toggled(bool)),this,SIGNAL(kineticScrollingEnabledChanged(bool)));
     connect(fontSizeComboBox,SIGNAL(activated(QString)),this,SLOT(setFontSize(QString)));
     connect(createPortable, SIGNAL(clicked(bool)), this, SLOT(createPortableAtPath()));
}

void Preferences::setFontSize(const QString size)
{
     QFont font;
     qreal pointSize = size.toFloat();
     font.setPointSize(pointSize);
     font.setFamily(fontComboBox->currentFont().family());
     fontComboBox->setFont(font);
}

void Preferences::showEvent(QShowEvent* show_pref)
{
     rootPathLabel->setText(settings->value("root_path").toString());
     backupPathLabel->setText(settings->value("backup_dir_path").toString());
     rootPath = settings->value("root_path").toString();
     originalRootPath = rootPath;
     QFont font;
     font.setFamily(settings->value("note_editor_font", "DejaVu Sans").toString());
     font.setPointSize(settings->value("note_editor_font_size", 10).toInt());
     fontComboBox->setFont(font);
     fontSizeComboBox->setCurrentIndex(fontSizeComboBox->findText(QString::number
                                        (settings->value("note_editor_font_size",10).toInt())));
     recentSpin->setValue(settings->value("Number_of_recent_Notes",5).toInt());

     QDialog::showEvent(show_pref);
}

void Preferences::saveSettings()
{
     if(!settings->isWritable())
       QMessageBox::warning(this,tr("Warning"),tr("Could not write settings!"));

     if(rootPath != originalRootPath){
       if(QMessageBox::question(this,tr("Keep old trash folder?"),
          tr("Do you want to keep the old trash folder associated with the path %1? "
            "(You will be able to see the old files in the trash again if you change "
            "back to the previous directory.)")
          .arg(QDir::toNativeSeparators(settings->value("backup_dir_path").toString())),
          QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
       {
            QList<QFileInfo> backups = QDir(settings->value("backup_dir_path").toString()).entryInfoList(QDir::Files);
            foreach(QFileInfo backup, backups)
              QFile(backup.absoluteFilePath()).remove();
            if(!QDir().rmdir(settings->value("backup_dir_path").toString()))
              QMessageBox::warning(this,tr("Couldn't delete trash folder"), tr("Could not delete the trash folder!"));
       }

       settings->setValue("root_path",rootPath);
       pathChanged();
     }

     settings->setValue("dont_quit_on_close", dontQuit->isChecked());
     settings->setValue("convert_notes", convertNotes->isChecked());
     settings->setValue("note_editor_default_size", QSize(sizeSpinWidth->value(),sizeSpinHeight->value()));
     settings->setValue("show_source", showSource->isChecked());
     settings->setValue("kinetic_scrolling", kineticScrolling->isChecked());
     settings->setValue("note_editor_font", fontComboBox->currentFont().family());
     settings->setValue("note_editor_font_size", fontComboBox->font().pointSize());
     settings->setValue("Number_of_recent_Notes", recentSpin->value());

     recentCountChanged(); //to update recent notes list

     accept();
}

QString Preferences::openDir()
{
     QString path;
     path = QFileDialog::getExistingDirectory(this,
                  tr("Open Directory"), rootPath, QFileDialog::ShowDirsOnly
                  | QFileDialog::DontResolveSymlinks);
     QFileInfo file(path);
     if(!file.isWritable() && !path.isEmpty()){
         QMessageBox::warning(this,tr("No Write Access"), tr("The path \"%1\" is not writable!").arg(QDir::toNativeSeparators(file.filePath())));
       return QString();
     }

     return QString(path + slash + "nobleNote");
}

void Preferences::setNewPaths()
{
     QString newPath = openDir();
     if(!newPath.isEmpty())
     {
          rootPath = newPath;
          rootPathLabel->setText(rootPath);
          backupPathLabel->setText(tr("(Will be updated after pressing OK.)"));
     }
}

void Preferences::createPortableAtPath()
{
     QString newPath = openDir();

     if(newPath.isEmpty())
       return;
     else
       QDir().mkpath(newPath);

    //copy all neccessary files for Windows
    #ifdef Q_OS_WIN32
     QStringList dirList = QDir(QCoreApplication::applicationDirPath()).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
     QStringList fileList = QDir(QCoreApplication::applicationDirPath()).entryList(QDir::Files);

     foreach(QString file, fileList)
     {
          //removing all files that are not .dll, .exe or .qm
          if(!file.contains(QRegEx("\.dll|\.exe|\.qm",Qt::CaseInsensitive)))
               fileList.remove(file);
          else //copy file located in the dir of the executable
               QFile(file).copy(newPath + slash + QFile(file).fileName());
     }

     //copy files located in dirList
     foreach(QString dir, dirList)
     {
          QString newDir = newPath + slash + QDir(dir).dirName();
          QDir().mkpath(newDir);
          QStringList dirFileList = QDir(newDir).entryList(QDir::Files);
          foreach(QString file, dirFileList)
               QFile(file).copy(newDir + slash + QFileInfo(file).fileName());
     }
    #else //as of Windows the executable was already included and copied above.
     QFile noblenote(QCoreApplication::applicationFilePath());
     noblenote.copy(newPath + slash + QFileInfo(QCoreApplication::applicationFilePath()).fileName());
    #endif

     //copy settings file
     QString newSettingsFilePath = newPath;
     QDir().mkpath(newSettingsFilePath);
     QString settingFileName = QFileInfo(settings->fileName()).fileName();
     QFile file(settings->fileName());
     file.copy(newSettingsFilePath + slash + settingFileName);
     QSettings newSettings(QString(newSettingsFilePath + slash +
                  settingFileName),QSettings::defaultFormat(),this); //create new settings

     //copy all notes to the new location
     QString newRootPath = newPath + slash +"notes";
     QList<QFileInfo> notebooks = QDir(settings->value("root_path").toString()).entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
     foreach(QFileInfo notebook, notebooks)
     {
          QDir().mkpath(newRootPath + slash + notebook.fileName());
          QList<QFileInfo> notes = QDir(notebook.absoluteFilePath()).entryInfoList(QDir::Files);
          foreach(QFileInfo note, notes)
               QFile(note.absoluteFilePath()).copy(
                     newRootPath + slash + notebook.fileName() + slash + note.fileName());
     }

     //copy all backups to the new location
     QString backupPath = newPath + slash +"backups";
     QDir().mkpath(backupPath);
     QList<QFileInfo> backups = QDir(settings->value("backup_dir_path").toString()).entryInfoList(QDir::Files);
     foreach(QFileInfo backup, backups)
          QFile(backup.absoluteFilePath()).copy(backupPath + slash + backup.fileName());

     //set portable and save new paths in the settings
     newSettings.setValue("isPortable",true);
     newSettings.setValue("root_path",newRootPath);
     newSettings.setValue("backup_dir_path",backupPath);
}
