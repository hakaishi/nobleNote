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

     //notePath = filePath;

     searchbarVisible = false; //initializing

     textEdit = new TextEdit(this);
     textDocument = new TextDocument(this);
     textEdit->setDocument(textDocument);
     textEdit->ensureCursorVisible();

     noteDescriptor_ = new NoteDescriptor(filePath,textEdit, textDocument,this); // must be constructed after TextDocument
     textEdit->setReadOnly(noteDescriptor_->readOnly());

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

void Note::setSearchBarText(QString str){ searchB->searchLine->setText(str); }
