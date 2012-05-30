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
#include <QSettings>
#include <textdocument.h>
#include "notedescriptor.h"

Note::Note(QString filePath, QWidget *parent) : QMainWindow(parent){

     setupUi(this);
     setAttribute(Qt::WA_DeleteOnClose);

     notePath = filePath;

     searchbarVisible = false; //initializing

     textEdit = new TextEdit(this);
     textDocument = new TextDocument(this);
     textEdit->setDocument(textDocument);
     textEdit->ensureCursorVisible();

     noteDescriptor_ = new NoteDescriptor(filePath,textDocument,this); // must be constructed after TextDocument

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

     restoreState(QSettings().value("Toolbars/state").toByteArray());

     searchB->setVisible(false);

     connect(noteDescriptor_,SIGNAL(close()),this,SLOT(close()));
     connect(textEdit,SIGNAL(signalFocusInEvent()),this->noteDescriptor_,SLOT(stateChange()));


     connect(searchB->searchLine, SIGNAL(textChanged(QString)), this, SLOT(selectNextExpression()));
     connect(searchB->findNext, SIGNAL(clicked(bool)), SLOT(selectNextExpression()));
     connect(searchB->findPrevious, SIGNAL(clicked(bool)), SLOT(selectPreviousExpression()));
}

Note::~Note(){ /*save_or_not(); */}

void Note::showEvent(QShowEvent* show_Note){
//     load();
     if(searchbarVisible)
       searchB->setVisible(true);

     QMainWindow::showEvent(show_Note);
}

void Note::closeEvent(QCloseEvent* close_Note){
     QSettings().setValue("Toolbars/state", saveState());

      if(textDocument->isModified())
        noteDescriptor_->stateChange();

     QMainWindow::closeEvent(close_Note);
}





// old ctor code
//     jTimer = new QTimer(this);
//     jTimer->setInterval(1000);
//     jTimer->setSingleShot(true);

//     timer = new QTimer(this);


//     connect(textEdit, SIGNAL(textChanged()), jTimer, SLOT(start()));
//     connect(textEdit, SIGNAL(sendFocusIn()), this, SLOT(save_or_not()));
//     connect(jTimer, SIGNAL(timeout()), this, SLOT(save_or_not()));
//     connect(timer, SIGNAL(timeout()), this, SLOT(save_or_not()));
//     connect(buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked(bool)),
//       this, SLOT(resetAll()));
// end old ctor code

//void Note::load(){
//     QFile note(notePath);
//     if(!note.open(QIODevice::ReadOnly))
//       return;
//     QTextStream nStream(&note);
//     text = nStream.readAll();
//     note.close();
//     QFileInfo noteInfo(notePath);
//     noteModified = noteInfo.lastModified();

//     setWindowTitle(noteInfo.fileName());
//     textEdit->setHtml(text);

//     QFile journal(journalsPath);
//     if(!journal.exists()){
//       if(!journal.open(QIODevice::WriteOnly | QIODevice::Truncate))
//         return;
//       QTextStream jStream(&journal);
//       jStream << text;
//     }
//     journal.close();
//     QFileInfo journalInfo(journalsPath);
//     journalModified = journalInfo.lastModified();
//}

//void Note::reload(){

//     QDir rootPath(QSettings().value("rootPath").toString());

//     QStringList files;

//     QDirIterator it(rootPath, QDirIterator::Subdirectories);
//     while(it.hasNext())
//       files << it.next();

//     QString foundFile;

//     for(int i = 0; i < files.size(); ++i){
//       QFile file(QDir(rootPath).absoluteFilePath(files[i]));
//       if(file.open(QIODevice::ReadOnly)){
//         QTextStream in(&file);
//         QString str = in.readAll();
//         if(str.contains(text)) //Here the UUID should be searched.
//           foundFile = files[i]; //This will be a problem if there is another note with the same content.
//       }
//     }

//     if(!foundFile.isEmpty()){
//       QFileInfo info(foundFile);
//       setWindowTitle(info.baseName());
//       notePath = info.filePath();
//       QString dirTrunc = info.filePath();
//       dirTrunc.remove( "/" + info.baseName());
//       dirTrunc.remove(QSettings().value("rootPath").toString() + "/");
//       journalsPath = QSettings().value("journalFolderPath").toString() + "/" +
//         dirTrunc + "_" + info.baseName() + ".journal";
//     }
//}

//void Note::saveAll(){
//     QFile note(notePath);
//     if(!note.open(QIODevice::WriteOnly | QIODevice::Truncate))
//       return;
//     QTextStream nStream(&note);
//     nStream << textEdit->toHtml();
//     note.close();
//     QFileInfo noteInfo(notePath);
//     noteModified = noteInfo.lastModified();

//     QFile journal(journalsPath);
//     if(!journal.open(QIODevice::WriteOnly | QIODevice::Truncate))
//       return;
//     QTextStream jStream(&journal);
//     jStream << textEdit->toHtml();
//     journal.close();
//     QFileInfo journalInfo(journalsPath);
//     journalModified = journalInfo.lastModified();

////      //test xml output
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

////     QFile f("C:/Users/Taiko/.nobleNote/notes/hand/easdf");
////     f.open(QFile::ReadOnly);

//     XmlNoteReader reader;
//     reader.setDevice(&buffer);
//     //reader.setFrame((new QTextDocument())->rootFrame());
//     qDebug() << reader.lastChange() << reader.lastMetadataChange() << reader.createDate();
//     qDebug() << reader.uuid() << "and static: ";
//     buffer.close();
//     buffer.open(QBuffer::ReadOnly);
//     qDebug() << XmlNoteReader::uuid(&buffer);
//}

//void Note::save_or_not(){
//     QFile note(notePath);
//     if(!note.exists()){
//       if(alreadyAsked)
//         return;
//       reload();
//       QFile note(notePath);
//       if(!note.exists()){
//         if(QMessageBox::warning(this,tr("Note doesn't exist"),
//            tr("This note does no longer exist. Do you want to "
//               "save it as a new one?"),
//            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes){
//           close();
//           alreadyAsked = true;
//           return;
//         }
//         else{
//           saveAll();
//           return;
//         }
//       }
//     }

//     QFileInfo noteInfo(notePath);
//     QFileInfo journalInfo(journalsPath);
//     if((noteModified != noteInfo.lastModified()) ||
//        (journalModified != journalInfo.lastModified())){
//       if(QMessageBox::warning(this,tr("Note was modified"),
//          tr("The note was modified. Do you want to save it "
//             "as a new one? \"No\" reloads the content."),
//          QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
//         load(); //try to reload file
//       else{
//         askForName();
//         return;
//       }
//     }

//     if(!note.open(QIODevice::ReadOnly))
//       return;
//     QTextStream nStream(&note);
//     QString content = nStream.readAll();
//     note.close();
//     if(content == textEdit->toHtml())
//       return; //don't save if text didn't change
//     else
//       saveAll();
//}

//void Note::resetAll(){
//     textEdit->setHtml(text);

//     QFile note(notePath);
//     if(!note.open(QIODevice::WriteOnly | QIODevice::Truncate))
//       return;
//     QTextStream stream(&note);
//     stream << text;
//     note.close();
//     QFileInfo noteInfo(notePath);
//     noteModified = noteInfo.lastModified();

//     QFile journal(journalsPath);
//     if(!journal.open(QIODevice::WriteOnly | QIODevice::Truncate))
//       return;
//     QTextStream jStream(&journal);
//     jStream << text;
//     journal.close();
//     QFileInfo journalInfo(journalsPath);
//     journalModified = journalInfo.lastModified();
//}

//void Note::askForName(){
//     newNote = new NewNoteName(this);
//     newNote->show();
//     connect(newNote, SIGNAL(newName()), this, SLOT(getNewName()));
//}

//void Note::getNewName(){
//     QFileInfo old(notePath);
//     QString newName = old.filePath();
//     newName.remove(old.baseName());
//     newName.append(newNote->lineEdit->text());
//     setWindowTitle(newNote->lineEdit->text());
//     notePath = newName;
//     QString dirTrunc = newName;
//     dirTrunc.remove( "/" + newNote->lineEdit->text());
//     dirTrunc.remove(QSettings().value("rootPath").toString() + "/");
//     journalsPath = QSettings().value("journalFolderPath").toString() + "/" +
//       dirTrunc + "_" + newNote->lineEdit->text() + ".journal";
//     saveAll();
//}

void Note::keyPressEvent(QKeyEvent *k){
     if((k->modifiers() == Qt::ControlModifier) && (k->key() == Qt::Key_F)){
       if(textEdit->textCursor().hasSelection()){
         searchB->searchLine->setText(textEdit->textCursor().selectedText());
         highlightText(textEdit->textCursor().selectedText());
       }
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
     highlightText(searchB->searchLine->text());
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
     highlightText(searchB->searchLine->text());
}

void Note::highlightText(QString str){
     highlighter = new Highlighter(textEdit->document());
     highlighter->expression = str;
     if(searchB->caseSensitiveBox->isChecked())
       highlighter->caseSensitive = true;
     else
       highlighter->caseSensitive = false;
     connect(searchB->closeSearch, SIGNAL(clicked(bool)), highlighter, SLOT(deleteLater()));
     connect(searchB->closeSearch, SIGNAL(clicked(bool)), textEdit, SLOT(setFocus()));
     connect(searchB->closeSearch, SIGNAL(clicked(bool)), searchB->searchLine, SLOT(clear()));
}
