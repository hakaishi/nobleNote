#include "preferences.h"
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>

Preferences::Preferences(QWidget *parent): QDialog(parent){
     setupUi(this);

     settings = new QSettings(this);
     pathLabel->setText(settings->value("rootPath").toString());

     dontQuit->setChecked(QSettings().value("Dont_quit_on_close",false).toBool());
     pSpin->setValue(QSettings().value("Save_notes_periodically",1).toInt());
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
       msgBox.setInformativeText(tr("nobleNote.conf is not writable!"));
       QTimer::singleShot(6000, &msgBox, SLOT(close()));
       msgBox.exec();
     }

     QFileInfo file(rootPath);
     if(!rootPath.isEmpty() && !file.isWritable()){
       QMessageBox msgBox;
       msgBox.setWindowTitle(tr("Warning"));
       msgBox.setIcon(QMessageBox::Critical);
       msgBox.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window);
       msgBox.setInformativeText(tr("The path \"%1\" is not writable!").arg(file.filePath()));
       QTimer::singleShot(6000, &msgBox, SLOT(close()));
       msgBox.exec();
       return;
     }

     settings->setValue("rootPath",rootPath);
     settings->setValue("Save_notes_periodically",pSpin->value());
     settings->setValue("Dont_quit_on_close", dontQuit->isChecked());

     sendPathChanged();
     accept();
}

void Preferences::openDir(){
     rootPath = QFileDialog::getExistingDirectory(this,
                  tr("Open Directory"), settings->value("rootPath").toString(), QFileDialog::ShowDirsOnly
                  | QFileDialog::DontResolveSymlinks);
     pathLabel->setText(rootPath);
}
