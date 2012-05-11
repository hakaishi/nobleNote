#include "preferences.h"
#include <QSettings>
#include <QDir>
#include <QMessageBox>
#include <QTimer>

Preferences::Preferences(QWidget *parent): QDialog(parent){
     setupUi(this);

     connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
}

Preferences::~Preferences(){}

void Preferences::saveSettings(){
     QString file(QDir::homePath() + "/.nobleNote/nobleNote.conf");
     QSettings settings(file, QSettings::IniFormat);
     if(!settings.isWritable()){
       QMessageBox msgBox;
       msgBox.setWindowTitle(tr("Warning"));
       msgBox.setIcon(QMessageBox::Critical);
       msgBox.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window);
       msgBox.setInformativeText(tr("nobleNote.conf ist not writable"));
       QTimer::singleShot(6000, &msgBox, SLOT(close()));
       msgBox.exec();
     }
     settings.setValue("Path to note folders",lineEdit->text());
     settings.setValue("Save notes periodically",pSpin->value());
     sendPathChanged();
}
