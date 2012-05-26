#include "note.h"
#include "textformattingtoolbar.h"
#include "newnotename.h"
#include "textedit.h"
#include "textsearchtoolbar.h"
#include "highlighter.h"
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
#include "xmlnotewriter.h"
#include "xmlnotereader.h"
#include <QBuffer>

Note::Note(QWidget *parent) : QMainWindow(parent){

     setupUi(this);

     setAttribute(Qt::WA_DeleteOnClose);

     textEdit = new TextEdit(this);
     textEdit->ensureCursorVisible();

     gridLayout->addWidget(textEdit, 0, 0, 1, 1);
     textEdit->setFocus();

     alreadyAsked = false;

     toolbar = new TextFormattingToolbar(textEdit,this);
     toolbar->setFocusPolicy(Qt::TabFocus);
     addToolBar(toolbar);

     addToolBarBreak(Qt::TopToolBarArea);

     searchB = new TextSearchToolbar(textEdit,this);
     searchB->setFocusPolicy(Qt::TabFocus);
     addToolBar(searchB);

     jTimer = new QTimer(this);
     jTimer->setInterval(1000);
     jTimer->setSingleShot(true);

     timer = new QTimer(this);


     connect(textEdit, SIGNAL(textChanged()), jTimer, SLOT(start()));
     connect(textEdit, SIGNAL(sendFocusIn()), this, SLOT(save_or_not()));
     connect(jTimer, SIGNAL(timeout()), this, SLOT(save_or_not()));
     connect(timer, SIGNAL(timeout()), this, SLOT(save_or_not()));
     connect(buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked(bool)),
       this, SLOT(resetAll()));
     connect(searchB->searchLine, SIGNAL(returnPressed()), this, SLOT(selectNextExpression()));
     connect(searchB->findNext, SIGNAL(clicked(bool)), SLOT(selectNextExpression()));
     connect(searchB->findPrevious, SIGNAL(clicked(bool)), SLOT(selectPreviousExpression()));
}

Note::~Note(){ save_or_not(); }

void Note::showEvent(QShowEvent* show_Note){
     load();
     QMainWindow::showEvent(show_Note);
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
     if(!journal.exists()){
       if(!journal.open(QIODevice::WriteOnly | QIODevice::Truncate))
         return;
       QTextStream jStream(&journal);
       jStream << text;
     }
     journal.close();
     QFileInfo journalInfo(journalsPath);
     journalModified = journalInfo.lastModified();
}

void Note::reload(){

     QDir rootPath(QSettings().value("rootPath").toString());

     QStringList files;

     QDirIterator it(rootPath, QDirIterator::Subdirectories);
     while(it.hasNext())
       files << it.next();

     QString foundFile;

     for(int i = 0; i < files.size(); ++i){
       QFile file(QDir(rootPath).absoluteFilePath(files[i]));
       if(file.open(QIODevice::ReadOnly)){
         QTextStream in(&file);
         QString str = in.readAll();
         if(str.contains(text)) //Here the UUID should be searched.
           foundFile = files[i]; //This will be a problem if there is another note with the same content.
       }
     }

     if(!foundFile.isEmpty()){
       QFileInfo info(foundFile);
       setWindowTitle(info.baseName());
       notesPath = info.filePath();
       QString dirTrunc = info.filePath();
       dirTrunc.remove( "/" + info.baseName());
       dirTrunc.remove(QSettings().value("rootPath").toString() + "/");
       journalsPath = QSettings().value("journalFolderPath").toString() + "/" +
         dirTrunc + "_" + info.baseName() + ".journal";
     }
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

//      //test xml output
//    QBuffer buffer;
//    buffer.open(QBuffer::ReadWrite);

//    XmlNoteWriter writer;
//    writer.setDevice(&buffer);
//    writer.setUuid(QUuid::createUuid());
//    writer.setLastChange(QDateTime::currentDateTime());
//    writer.setFrame(textEdit->document()->rootFrame());
//    writer.write();
//     qDebug() << buffer.data();

//     buffer.close();
//     buffer.open(QBuffer::ReadWrite);

//     QFile f("C:/Users/Taiko/a.note");
//     f.open(QFile::ReadOnly);

//     XmlNoteReader reader;
//     reader.setDevice(&buffer);
//     reader.setFrame((new QTextDocument())->rootFrame());
//     reader.read();
//     qDebug() << reader.uuid() << "and static: ";
//     buffer.close();
//     buffer.open(QBuffer::ReadOnly);
//     qDebug() << XmlNoteReader::uuid(&buffer);
}

void Note::save_or_not(){
     QFile note(notesPath);
     if(!note.exists()){
       if(alreadyAsked)  
         return;
       reload();
       QFile note(notesPath);
       if(!note.exists()){
         if(QMessageBox::warning(this,tr("Note doesn't exist"),
            tr("This note does no longer exist. Do you want to "
               "save it as a new one?"),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes){
           close();
           alreadyAsked = true;
           return;
         }
         else{
           saveAll();
           return;
         }
       }
     }

     QFileInfo noteInfo(notesPath);
     QFileInfo journalInfo(journalsPath);
     if((noteModified != noteInfo.lastModified()) ||
        (journalModified != journalInfo.lastModified())){
       if(QMessageBox::warning(this,tr("Note was modified"),
          tr("The note was modified. Do you want to save it "
             "as a new one? \"No\" reloads the content."),
          QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
         load(); //try to reload file
       else{
         askForName();
         return;
       }
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

void Note::askForName(){
     newNote = new NewNoteName(this);
     newNote->show();
     connect(newNote, SIGNAL(newName()), this, SLOT(getNewName()));
}

void Note::getNewName(){
     QFileInfo old(notesPath);
     QString newName = old.filePath();
     newName.remove(old.baseName());
     newName.append(newNote->lineEdit->text());
     setWindowTitle(newNote->lineEdit->text());
     notesPath = newName;
     QString dirTrunc = newName;
     dirTrunc.remove( "/" + newNote->lineEdit->text());
     dirTrunc.remove(QSettings().value("rootPath").toString() + "/");
     journalsPath = QSettings().value("journalFolderPath").toString() + "/" +
       dirTrunc + "_" + newNote->lineEdit->text() + ".journal";
     saveAll();
}

void Note::keyPressEvent(QKeyEvent *k){
     if((k->modifiers() == Qt::ControlModifier) && (k->key() == Qt::Key_F)){
       if(!searchB->isVisible())
         searchB->setVisible(true);
         searchB->searchLine->setFocus();
     }
}

void Note::selectNextExpression(){
     if(searchB->caseSensitiveBox->isChecked()){
       if(!textEdit->find(searchB->searchLine->text(), QTextDocument::FindCaseSensitively)){
         textEdit->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
         textEdit->find(searchB->searchLine->text(), QTextDocument::FindCaseSensitively);
       }
     }
     else{
       if(!textEdit->find(searchB->searchLine->text())){
         textEdit->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
         textEdit->find(searchB->searchLine->text());
       }
     }
     highlightText();
}

void Note::selectPreviousExpression(){
     if(searchB->caseSensitiveBox->isChecked()){
       if(!textEdit->find(searchB->searchLine->text(), QTextDocument::FindCaseSensitively |
           QTextDocument::FindBackward)){
         textEdit->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
         textEdit->find(searchB->searchLine->text(), QTextDocument::FindCaseSensitively |
           QTextDocument::FindBackward);
       }
     }
     else{
       if(!textEdit->find(searchB->searchLine->text(), QTextDocument::FindBackward)){
         textEdit->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
         textEdit->find(searchB->searchLine->text(), QTextDocument::FindBackward);
       }
     }
     highlightText();
}

void Note::highlightText(){
     highlighter = new Highlighter(textEdit->document());
     highlighter->expression = searchB->searchLine->text();
     if(searchB->caseSensitiveBox->isChecked())
       highlighter->caseSensitive = true;
     else
       highlighter->caseSensitive = false;
     connect(searchB->closeSearch, SIGNAL(clicked(bool)), highlighter, SLOT(deleteLater()));
     connect(searchB->closeSearch, SIGNAL(clicked(bool)), textEdit, SLOT(setFocus()));
}
