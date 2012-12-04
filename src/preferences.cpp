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
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <QDesktopServices>

Preferences::Preferences(QWidget *parent): QDialog(parent)
{
     setupUi(this);

     settings = new QSettings(this);

     QFontDatabase db;
     foreach(int size, db.standardSizes())
       fontSizeComboBox->addItem(QString::number(size));

     dontQuit->setChecked(settings->value("dont_quit_on_close",false).toBool());
     convertNotes->setChecked(settings->value("convert_notes",true).toBool());
     showSource->setChecked(settings->value("show_source", false).toBool());
     kineticScrolling->setChecked(settings->value("kinetic_scrolling", false).toBool());
     sizeSpinHeight->setValue(settings->value("note_editor_default_size",QSize(335,250)).toSize().height());
     sizeSpinWidth->setValue(settings->value("note_editor_default_size",QSize(335,250)).toSize().width());


     connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
     connect(browseButton, SIGNAL(clicked(bool)), this, SLOT(openDir()));
     connect(kineticScrolling,SIGNAL(toggled(bool)),this,SIGNAL(kineticScrollingEnabledChanged(bool)));
     connect(fontSizeComboBox,SIGNAL(activated(QString)),this,SLOT(setFontSize(QString)));
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
     pathLabel->setText(settings->value("root_path").toString());
     rootPath = settings->value("root_path").toString();
     originalRootPath = rootPath;
     QFont font;
     font.setFamily(settings->value("note_editor_font", "DejaVu Sans").toString());
     font.setPointSize(settings->value("note_editor_font_size", 10).toInt());
     fontComboBox->setFont(font);
     fontSizeComboBox->setCurrentIndex(fontSizeComboBox->findText(QString::number
                                        (settings->value("note_editor_font_size",10).toInt())));

     QDialog::showEvent(show_pref);
}

void Preferences::saveSettings()
{
     if(!settings->isWritable())
       QMessageBox::warning(this,tr("Warning"),tr("Could not write settings!"));

     if(rootPath != originalRootPath){
       if(QMessageBox::question(this,tr("Keep old trash folder?"),
          tr("Do you want to keep the old trash folder associated with the path %2? "
            "(You will be able to see the old files in the trash again if you change "
            "back to the previous directory.)")
          .arg(QApplication::applicationName(),QDir::toNativeSeparators(
               settings->value("backup_dir_path").toString())),
          QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
       {
            QList<QFileInfo> backups = QDir(settings->value("backup_dir_path").toString()).entryInfoList(QDir::Files);
            foreach(QFileInfo backup, backups)
              QFile(backup.absoluteFilePath()).remove();
            if(!QDir().rmdir(settings->value("backup_dir_path").toString()))
              QMessageBox::warning(this,tr("Couldn't delete backup folder"), tr("Could not delete the backup folder!"));
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

     accept();
}

void Preferences::openDir()
{
     QString path;
     path = QFileDialog::getExistingDirectory(this,
                  tr("Open Directory"), rootPath, QFileDialog::ShowDirsOnly
                  | QFileDialog::DontResolveSymlinks);
     QFileInfo file(path);
     if(!file.isWritable() && !path.isEmpty()){
         QMessageBox::warning(this,tr("No Write Access"), tr("The path \"%1\" is not writable!").arg(QDir::toNativeSeparators(file.filePath())));
       return;
     }
     if(!path.isEmpty())
     {
          rootPath = path;
          pathLabel->setText(rootPath);
     }
}
