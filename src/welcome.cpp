#include "welcome.h"
#include "lineedit.h"
#include <QFileDialog>
#include <QSettings>

Welcome::Welcome(QWidget *parent): QDialog(parent){
     setupUi(this);

     path = new LineEdit(this);
     path->setText(QDir::homePath() + "/.nobleNote");
     gridLayout->addWidget(path, 3, 0, 1, 1);

     connect(browse, SIGNAL(clicked(bool)), this, SLOT(openDir()));
     connect(this, SIGNAL(accepted()), this, SLOT(setRootDir()));
     connect(this, SIGNAL(rejected()), this, SLOT(setRootDir()));
}

void Welcome::openDir(){
     QString str = QFileDialog::getExistingDirectory(this,
       tr("Choose a directory"), "/home",
       QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
     if(str != "")
       path->setText(str);
     else
       path->setText(QDir::homePath() + "/.nobleNote");
}

void Welcome::setRootDir(){
     if(path->text() == "")
       QSettings().setValue("rootPath", QDir::homePath() + "/.nobleNote");
     else
       QSettings().setValue("rootPath", path->text());
}
