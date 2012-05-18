#include "note.h"
#include "xmlnote.h"
#include "textformattingtoolbar.h"
#include <QFile>
#include <QPushButton>
#include <QDir>
#include <QToolBar>
#include <QColorDialog>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QTextBlock>
#include <QTextFragment>
#include <QMessageBox>
#include <QSettings>
#include <QDirIterator>
#include <QDebug>

Note::Note(QWidget *parent) : QMainWindow(parent){

     setupUi(this);

     setFocusPolicy(Qt::StrongFocus);

     toolbar = new TextFormattingToolbar(textEdit,this);
     addToolBar(toolbar);

     jTimer = new QTimer(this);
     jTimer->setInterval(1000);
     jTimer->setSingleShot(true);

     timer = new QTimer(this);



     connect(textEdit, SIGNAL(textChanged()), jTimer, SLOT(start()));
     connect(jTimer, SIGNAL(timeout()), this, SLOT(save_or_not()));
     connect(timer, SIGNAL(timeout()), this, SLOT(save_or_not()));
     connect(buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked(bool)),
       this, SLOT(resetAll()));
}

Note::~Note(){ checkAndSaveFile(); }

void Note::showEvent(QShowEvent* show_Note){
     load();
     QWidget::showEvent(show_Note);
}

void Note::focusInEvent(QFocusEvent *ef){
     save_or_not(); //checks if file was modified and reloads if that's the case
                    //and also checks if the file still exists
qDebug()<<"focus in";
     QWidget::focusInEvent(ef);
}

void Note::load(){
     QFile note(notesPath);
     if(!note.open(QIODevice::ReadOnly))
       return;
     QTextStream nStream(&note);
     text = nStream.readAll();
     note.close();
     QFileInfo noteInfo(notesPath);
     noteModified = noteInfo.lastModified();

     setWindowTitle(noteInfo.fileName());
     textEdit->setHtml(text);

     QFile journal(journalsPath);
     if(!journal.open(QIODevice::WriteOnly | QIODevice::Truncate))
       return;
     if(!journal.exists()){
       QTextStream jStream(&journal);
       jStream << text;
     }
     journal.close();
     QFileInfo journalInfo(journalsPath);
     journalModified = journalInfo.lastModified();
}

void Note::reload(){
     QSettings settings;
     QDir rootPath(settings.value("rootPath").toString());

     QStringList files;

     QDirIterator it(rootPath, QDirIterator::Subdirectories);
     while(it.hasNext())
       files << it.next();

     QString foundFile;

     for(int i = 0; i < files.size(); ++i){
       QFile file(QDir(rootPath).absoluteFilePath(files[i]));
       if(file.open(QIODevice::ReadOnly)){
         QTextStream in(&file);
         QString line = in.readAll();
         if(line.contains(text))
           foundFile = files[i];
       }
     }

     QFileInfo info(foundFile);
     setWindowTitle(info.baseName());
     notesPath = info.filePath();
}

void Note::saveAll(){
     QFile note(notesPath);
     if(!note.open(QIODevice::WriteOnly | QIODevice::Truncate))
       return;
     QTextStream nStream(&note);
     nStream << textEdit->toHtml();
     note.close();
     QFileInfo noteInfo(notesPath);
     noteModified = noteInfo.lastModified();

     QFile journal(journalsPath);
     if(!journal.open(QIODevice::WriteOnly | QIODevice::Truncate))
       return;
     QTextStream jStream(&journal);
     jStream << textEdit->toHtml();
     journal.close();
     QFileInfo journalInfo(journalsPath);
     journalModified = journalInfo.lastModified();

qDebug()<<"saved";

     // test xml output
//    QString xmlOutput;
//    QString noteName ="foo bar";
//    XmlNote writer(&xmlOutput);
//    writer.setFrame(textEdit->document()->rootFrame());
//    writer.write();
//     qDebug() << xmlOutput;
}

void Note::save_or_not(){
     QFile note(notesPath);
     if(!note.exists()){
       reload();
       QFile note(notesPath);
       if(!note.exists()){
         if(QMessageBox::warning(this,tr("Note doesn't exist"),
            tr("Do you want to save this note as a new one?"),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
           close();
         else
           saveAll();
       }
     }
     else
       checkAndSaveFile();
}

void Note::checkAndSaveFile(){
     QFile note(notesPath);
     QFileInfo noteInfo(notesPath);
     QFileInfo journalInfo(journalsPath);
     if((noteModified != noteInfo.lastModified()) ||
        (journalModified != journalInfo.lastModified())){
       load(); //try to reload file
       return;
     }

     if(!note.open(QIODevice::ReadOnly))
       return;
     QTextStream nStream(&note);
     QString content = nStream.readAll();
     note.close();
     if(content == textEdit->toHtml())
       return; //don't save if text didn't change
     else
       saveAll();
}

void Note::resetAll(){
     textEdit->setHtml(text);

     QFile note(notesPath);
     if(!note.open(QIODevice::WriteOnly | QIODevice::Truncate))
       return;
     QTextStream stream(&note);
     stream << text;
     note.close();
     QFileInfo noteInfo(notesPath);
     noteModified = noteInfo.lastModified();

     QFile journal(journalsPath);
     if(!journal.open(QIODevice::WriteOnly | QIODevice::Truncate))
       return;
     QTextStream jStream(&journal);
     jStream << text;
     journal.close();
     QFileInfo journalInfo(journalsPath);
     journalModified = journalInfo.lastModified();
}

void Note::keyPressEvent(QKeyEvent *k){
     if((k->modifiers() == Qt::ControlModifier) && (k->key() == Qt::Key_F)){
//TODO:text search.
     }
}


