#include "preferences.h"
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>

Preferences::Preferences(QWidget *parent): QDialog(parent){
     setupUi(this);

     settings = new QSettings(this);
     pathLabel->setText(settings->value("rootPath").toString());
     rootPath = settings->value("rootPath").toString();

     dontQuit->setChecked(settings->value("Dont_quit_on_close",false).toBool());
     convertNotes->setChecked(settings->value("convert_notes",true).toBool());
     connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
     connect(browseButton, SIGNAL(clicked(bool)), this, SLOT(openDir()));
}

Preferences::~Preferences(){}

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

     settings->setValue("rootPath",rootPath);
     settings->setValue("noteDirPath",rootPath + "/notes");
     settings->setValue("backupDirPath",rootPath + "/backups");
     settings->setValue("Dont_quit_on_close", dontQuit->isChecked());
     settings->setValue("convert_notes", convertNotes->isChecked());

     sendPathChanged();
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
