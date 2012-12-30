/* nobleNote, a note taking application
 * Copyright (C) 2012 Christian Metscher <hakaishi@web.de>,
                      Fabian Deuchler <Taiko000@gmail.com>

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.

 * nobleNote is licensed under the MIT, see `http://copyfree.org/licenses/mit/license.txt'.
 */

#include "note.h"
#include "textformattingtoolbar.h"
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
//#include "flickcharm.h"

Note::Note(QString filePath, QWidget *parent) : QMainWindow(parent){

     setupUi(this);
     setAttribute(Qt::WA_DeleteOnClose);

     textBrowser = new TextBrowser(this);
     textDocument = new TextDocument(this);
     textBrowser->setDocument(textDocument);
     textBrowser->ensureCursorVisible();
     textBrowser->setTabStopWidth( fontMetrics().width(" ") * 4); // set tab size to 4 for android compatibility

//     FlickCharm * flickCharm = new FlickCharm(this);
//     flickCharm->activateOn(textBrowser);


     toolbar = new TextFormattingToolbar(textBrowser,this);
     toolbar->setFocusPolicy(Qt::TabFocus);
     addToolBar(toolbar);
     addToolBarBreak(Qt::TopToolBarArea);

     QFont font;
     font.setFamily(QSettings().value("note_editor_font", "DejaVu Sans").toString());
     font.setPointSize(QSettings().value("note_editor_font_size", 10).toInt());
     textDocument->setDefaultFont(font);
     toolbar->setFont(font);

     noteDescriptor_ = new NoteDescriptor(filePath,textBrowser, textDocument,this); // must be constructed after TextDocument
     textBrowser->setReadOnly(noteDescriptor_->readOnly());
     textBrowser->setTextInteractionFlags(textBrowser->textInteractionFlags()|
                                             Qt::LinksAccessibleByMouse);

     gridLayout->addWidget(textBrowser, 0, 0, 1, 1);
     textBrowser->setFocus();

     searchBar = new TextSearchToolbar(textBrowser,this);
     searchBar->setFocusPolicy(Qt::TabFocus);
     addToolBar(searchBar);

     restoreState(QSettings().value("Toolbars/state").toByteArray());

     connect(noteDescriptor_,SIGNAL(close()),this,SLOT(close()));
     connect(textBrowser,SIGNAL(signalFocusInEvent()),this->noteDescriptor_,SLOT(stateChange()));
}

void Note::highlightText(const QString &str)
{
    searchBar->highlightText(str);
}

void Note::showEvent(QShowEvent* event){
     QSize defaultSize(QSettings().value("note_editor_default_size",QSize(335,250)).toSize());
     QSize size = QSettings().value("Notes/"+noteDescriptor_->uuid().toString()+"_size", defaultSize).toSize();
     resize(size);

     QMainWindow::showEvent(event);
}

void Note::closeEvent(QCloseEvent* close_Note){
     QSettings().setValue("Notes/"+noteDescriptor_->uuid().toString()+"_size", size());
     QSettings().setValue("Toolbars/state", saveState());
     QSettings().setValue("Notes/"+noteDescriptor_->uuid().toString()+"_cursor_position",
        textBrowser->textCursor().position());
     QStringList savedOpenNoteList = QSettings().value("open_notes").toStringList();
     savedOpenNoteList.removeOne(noteDescriptor_->filePath());
     QSettings().setValue("open_notes", savedOpenNoteList);

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
         searchBar->setText(textBrowser->textCursor().selectedText());
       }
       if(!searchBar->isVisible())
         searchBar->setVisible(true);
         searchBar->searchLine()->setFocus();
     }
}

void Note::keyReleaseEvent(QKeyEvent *k){
     Q_UNUSED(k);
     textBrowser->setReadOnly(noteDescriptor_->readOnly());
     textBrowser->setTextInteractionFlags(textBrowser->textInteractionFlags()|
                                            Qt::LinksAccessibleByMouse);
}



void Note::setSearchBarText(QString str)
{
    searchBar->setText(str);
    searchBar->selectNextExpression();
    searchBar->setVisible(true);
}
