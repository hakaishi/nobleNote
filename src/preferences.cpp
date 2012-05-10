#include "preferences.h"
#include <QSettings>
#include <QDir>
#include <QTextStream>

Preferences::Preferences(QWidget *parent): QDialog(parent){
     setupUi(this);

     connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
}

Preferences::~Preferences(){}

void Preferences::saveSettings(){
     QString file(QDir::homePath() + "/.nobleNote/nobleNote.conf");
     QSettings settings(file, QSettings::IniFormat);
     if(!settings.isWritable()){
       QTextStream myOutput;
       myOutput << "W: nobelNote.conf is not writable!" << endl;
     }
     settings.setValue("Path to note folders",lineEdit->text());
     sendPathChanged();
}
