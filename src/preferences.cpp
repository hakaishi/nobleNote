#include "preferences.h"
#include <QSettings>
#include <QDir>
#include <QMessageBox>
#include <QTimer>

Preferences::Preferences(QWidget *parent): QDialog(parent){
     setupUi(this);

     dontQuit->setChecked(QSettings().value("Dont_quit_on_close",false).toBool());
     pSpin->setValue(QSettings().value("Save_notes_periodically",1).toInt());
     connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
}

Preferences::~Preferences(){}

void Preferences::saveSettings(){
     QSettings settings;
     if(!settings.isWritable()){
       QMessageBox msgBox;
       msgBox.setWindowTitle(tr("Warning"));
       msgBox.setIcon(QMessageBox::Critical);
       msgBox.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window);
       msgBox.setInformativeText(tr("nobleNote.conf is not writable!"));
       QTimer::singleShot(6000, &msgBox, SLOT(close()));
       msgBox.exec();
     }

     QFileInfo file(lineEdit->text());
     if(!lineEdit->text().isEmpty() && !file.isWritable()){
       QMessageBox msgBox;
       msgBox.setWindowTitle(tr("Warning"));
       msgBox.setIcon(QMessageBox::Critical);
       msgBox.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window);
       msgBox.setInformativeText(tr("The path \"%1\" is not writable!").arg(file.filePath()));
       QTimer::singleShot(6000, &msgBox, SLOT(close()));
       msgBox.exec();
       return;
     }

     settings.setValue("rootPath",lineEdit->text());
     settings.setValue("Save_notes_periodically",pSpin->value());
     settings.setValue("Dont_quit_on_close", dontQuit->isChecked());

     sendPathChanged();
     accept();
}
