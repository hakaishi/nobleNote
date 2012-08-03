#include "note.h"
#include "textformattingtoolbar.h"
#include "newnotename.h"
#include "textbrowser.h"
#include "textsearchtoolbar.h"
#include "highlighter.h"
#include "textdocument.h"
#include "notedescriptor.h"
#include <QFile>
#include <QPushButton>
#include <QDir>
#include <QToolBar>
#include <QColorDialog>
#include <QSettings>

Note::Note(QString filePath, QWidget *parent) : QMainWindow(parent){

     setupUi(this);
     setAttribute(Qt::WA_DeleteOnClose);

     //notePath = filePath;

     searchbarVisible = false; //initializing

     buttonBox = new QDialogButtonBox(this);
     buttonBox->setStandardButtons(QDialogButtonBox::Reset);
     gridLayout->addWidget(buttonBox, 1, 0, 1, 1);

     textBrowser = new TextBrowser(this);
     textDocument = new TextDocument(this);
     textBrowser->setDocument(textDocument);
     textBrowser->ensureCursorVisible();

     noteDescriptor_ = new NoteDescriptor(filePath,textBrowser, textDocument,this); // must be constructed after TextDocument
     textBrowser->setReadOnly(noteDescriptor_->readOnly());
     textBrowser->setTextInteractionFlags(textBrowser->textInteractionFlags()|
                                             Qt::LinksAccessibleByMouse);

     gridLayout->addWidget(textBrowser, 0, 0, 1, 1);
     textBrowser->setFocus();

     alreadyAsked = false;

     toolbar = new TextFormattingToolbar(textBrowser,this);
     toolbar->setFocusPolicy(Qt::TabFocus);
     addToolBar(toolbar);

     addToolBarBreak(Qt::TopToolBarArea);

     searchB = new TextSearchToolbar(textBrowser,this);
     searchB->setFocusPolicy(Qt::TabFocus);
     addToolBar(searchB);

     restoreState(QSettings().value("Toolbars/state").toByteArray());

     searchB->setVisible(false);

     connect(noteDescriptor_,SIGNAL(close()),this,SLOT(close()));
     connect(buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked(bool)),
       this, SLOT(undoAll()));
     connect(textBrowser,SIGNAL(signalFocusInEvent()),this->noteDescriptor_,SLOT(stateChange()));


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
     if(k->modifiers() == Qt::ControlModifier){
       textBrowser->setTextInteractionFlags(Qt::LinksAccessibleByMouse |
                                              Qt::LinksAccessibleByKeyboard);
       textBrowser->setReadOnly(noteDescriptor_->readOnly());
     }

     if((k->modifiers() == Qt::ControlModifier) && (k->key() == Qt::Key_F)){
       if(textBrowser->textCursor().hasSelection()){
         searchB->searchLine->setText(textBrowser->textCursor().selectedText());
         highlightText(textBrowser->textCursor().selectedText());
       }
       if(!searchB->isVisible())
         searchB->setVisible(true);
         searchB->searchLine->setFocus();
     }
}

void Note::keyReleaseEvent(QKeyEvent *k){
     textBrowser->setReadOnly(noteDescriptor_->readOnly());
     textBrowser->setTextInteractionFlags(textBrowser->textInteractionFlags()|
                                            Qt::LinksAccessibleByMouse);
}

void Note::undoAll(){
     while(textBrowser->document()->availableUndoSteps() > 0)
       textBrowser->undo();
}

void Note::selectNextExpression(){
     if(searchB->caseSensitiveBox->isChecked()){
       if(!textBrowser->find(searchB->searchLine->text(), QTextDocument::FindCaseSensitively)){
         textBrowser->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
         textBrowser->find(searchB->searchLine->text(), QTextDocument::FindCaseSensitively);
       }
     }
     else{
       if(!textBrowser->find(searchB->searchLine->text())){
         textBrowser->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
         textBrowser->find(searchB->searchLine->text());
       }
     }
     highlightText(searchB->searchLine->text());
}

void Note::selectPreviousExpression(){
     if(searchB->caseSensitiveBox->isChecked()){
       if(!textBrowser->find(searchB->searchLine->text(), QTextDocument::FindCaseSensitively |
           QTextDocument::FindBackward)){
         textBrowser->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
         textBrowser->find(searchB->searchLine->text(), QTextDocument::FindCaseSensitively |
           QTextDocument::FindBackward);
       }
     }
     else{
       if(!textBrowser->find(searchB->searchLine->text(), QTextDocument::FindBackward)){
         textBrowser->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
         textBrowser->find(searchB->searchLine->text(), QTextDocument::FindBackward);
       }
     }
     highlightText(searchB->searchLine->text());
}

void Note::highlightText(QString str){
     highlighter = new Highlighter(textBrowser->document());
     highlighter->expression = str;
     if(searchB->caseSensitiveBox->isChecked())
       highlighter->caseSensitive = true;
     else
       highlighter->caseSensitive = false;
     connect(searchB->closeSearch, SIGNAL(clicked(bool)), highlighter, SLOT(deleteLater()));
     connect(searchB->closeSearch, SIGNAL(clicked(bool)), textBrowser, SLOT(setFocus()));
     connect(searchB->closeSearch, SIGNAL(clicked(bool)), searchB->searchLine, SLOT(clear()));
}

void Note::setSearchBarText(QString str){ searchB->searchLine->setText(str); }
