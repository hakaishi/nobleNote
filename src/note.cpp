#include "note.h"
#include <QDebug>
#include <QFile>
#include <QPushButton>
#include <QDir>

Note::Note(QWidget *parent) : QDialog (parent){

     setupUi(this);

     //TODO:Make text formating possible

     timer = new QTimer(this);
     timer->setInterval(1000);
     timer->setSingleShot(true);

     connect(textEdit, SIGNAL(textChanged()), timer, SLOT(start()));
     connect(timer, SIGNAL(timeout()), this, SLOT(saveNote()));
     connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked(bool)), this, SLOT(saveNote()));
     connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked(bool)), this, SLOT(dontSave()));

}

Note::~Note(){}

void Note::saveNote(){
       QFile file(notesPath);
       if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
         return;
       QTextStream stream(&file);
       stream << textEdit->toHtml();
       file.close();
       //TODO 3: create real notes (so far only journal)
}

void Note::dontSave(){
     //TODO: restore to the state when file was first opened.
       QFile file(notesPath);
       if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
         return;
       QTextStream stream(&file);
       stream << text;
       file.close();
}

void Note::showEvent(QShowEvent* show_Note){
     textEdit->setHtml(text);
     QWidget::showEvent(show_Note);
}

void Note::closeEvent(QCloseEvent* close_Note){
     dontSave();
     QWidget::closeEvent(close_Note);
}
