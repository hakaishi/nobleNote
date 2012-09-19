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
#include "backup.h"
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>

Preferences::Preferences(QWidget *parent): QDialog(parent){
     setupUi(this);

     settings = new QSettings(this);

     dontQuit->setChecked(settings->value("Dont_quit_on_close",false).toBool());
     convertNotes->setChecked(settings->value("convert_notes",true).toBool());
     showSource->setChecked(settings->value("Show_source", false).toBool());
     sizeSpinHeight->setValue(settings->value("note_editor_default_size",QSize(335,250)).toSize().height());
     sizeSpinWidth->setValue(settings->value("note_editor_default_size",QSize(335,250)).toSize().width());

     connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
     connect(browseButton, SIGNAL(clicked(bool)), this, SLOT(openDir()));
     connect(restoreBackupsButton, SIGNAL(clicked(bool)), this, SLOT(showBackupWindow()));
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
#ifdef Q_WS_X11
       settings->setValue("backupDirPath",QDir::homePath() + "/.local/share/nobleNote/backups");
#else
       settings->setValue("backupDirPath",QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/nobleNote/backups");
#endif
       sendPathChanged();
     }

     settings->setValue("Dont_quit_on_close", dontQuit->isChecked());
     settings->setValue("convert_notes", convertNotes->isChecked());
     settings->setValue("note_editor_default_size", QSize(sizeSpinWidth->value(),sizeSpinHeight->value()));
     settings->setValue("Show_source", showSource->isChecked());

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

void Preferences::showBackupWindow(){
     backup = new Backup(this);
     backup->show();
}
