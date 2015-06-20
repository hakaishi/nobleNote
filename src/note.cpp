/* nobleNote, a note taking application
 * Copyright (C) 2015 Christian Metscher <hakaishi@web.de>,
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
#include <QDesktopWidget>
#include <QtConcurrentRun>
//#include "flickcharm.h"

Note::Note(QString filePath, QWidget *parent) : QMainWindow(parent){

     setupUi(this);
     setAttribute(Qt::WA_DeleteOnClose);

     showAfterLoading_ = false; // do not show() after loading settings
     textBrowser = new TextBrowser(this);
     textDocument = new TextDocument(this);
     textBrowser->setDocument(textDocument);
     textBrowser->ensureCursorVisible();
     textBrowser->setTabStopWidth( fontMetrics().width(" ") * 4); // set tab size to 4 for android compatibility

     showHideToolbars = new QAction(this);
     showHideToolbars->setShortcut(Qt::CTRL + Qt::Key_T);
     showHideToolbars->setPriority(QAction::LowPriority);
     connect(showHideToolbars, SIGNAL(triggered()), this, SLOT(showOrHideToolbars()));
     textBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
     connect(textBrowser,SIGNAL(customContextMenuRequested(const QPoint&)),
             this,SLOT(showContextMenu(const QPoint &)));


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

     restoreState(QSettings().value("Note_window_and_toolbar/state").toByteArray());

     if(QSettings().value("Notes/"+noteDescriptor_->uuid().toString()+"_window_position").isValid())
        restoreGeometry(QSettings().value("Notes/"+noteDescriptor_->uuid().toString()+"_window_position").toByteArray());
     else // center in desktop if there's no saved position
        move(QApplication::desktop()->screen()->rect().center() - rect().center());


     connect(noteDescriptor_,SIGNAL(close()),this,SLOT(close()));
     connect(textBrowser,SIGNAL(signalFocusInEvent()),this->noteDescriptor_,SLOT(stateChange()));
     connect(noteDescriptor_,SIGNAL(loadFinished(HtmlNoteReader*)),this,SLOT(loadSizeAndShow()),Qt::QueuedConnection);
}

void Note::highlightText(const QString &str)
{
     searchBar->highlightText(str);
}


void Note::loadSizeAndShow()
{
    QSize defaultSize(QSettings().value("note_editor_default_size",QSize(335,250)).toSize());
    QSize size = QSettings().value("Notes/"+noteDescriptor_->uuid().toString()+"_size", defaultSize).toSize();
    resize(size);
    if(showAfterLoading_)
    {
        show();
    }
}

void Note::closeEvent(QCloseEvent* close_Note)
{
    QVariantList variantList;
    noteDescriptor_->stateChange();

    // collect all stuff that requires the gui thread and pointers that are soon not longer valid
    variantList << size() << saveState() << saveGeometry() << textBrowser->textCursor().position() <<
                           noteDescriptor()->uuid().toString() << noteDescriptor()->filePath();

   // run it asynchronously to avoid blocking the gui thread, when he destroys the window
     future_ = QtConcurrent::run(this, &Note::saveWindowState,variantList);
     QMainWindow::closeEvent(close_Note);
}

/*static */ void Note::addToOpenNoteList(QString path)
{
     QStringList savedOpenNoteList;
    if(QSettings().value("open_notes").isValid())
      savedOpenNoteList = QSettings().value("open_notes").toStringList();
    savedOpenNoteList.append(path);
    QSettings().setValue("open_notes",savedOpenNoteList);
}

void Note::saveWindowState(QVariantList variantList)
{
    // see closeEvent() for the order of items in this variant list
    QUuid uuid = QUuid(variantList[4].toString()); // qt 4.8 does not support QUUId in QVariant
    QSettings().setValue("Notes/"+uuid.toString()+"_size", variantList[0].toSize());
    QSettings().setValue("Note_window_and_toolbar/state", variantList[1].toByteArray());
    QSettings().setValue("Notes/"+uuid.toString()+"_window_position", variantList[2].toByteArray());
    QSettings().setValue("Notes/"+uuid.toString()+"_cursor_position",variantList[3].toInt()/*textCursorPosition*/);
    QStringList savedOpenNoteList = QSettings().value("open_notes").toStringList();
    savedOpenNoteList.removeOne(variantList[5].toString());
    QSettings().setValue("open_notes", savedOpenNoteList);



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

     if((k->modifiers() == Qt::ControlModifier) && (k->key() == Qt::Key_T))
       showOrHideToolbars();

     QMainWindow::keyPressEvent(k);
}

void Note::keyReleaseEvent(QKeyEvent *k){
     textBrowser->setReadOnly(noteDescriptor_->readOnly());
     textBrowser->setTextInteractionFlags(textBrowser->textInteractionFlags()|
                                            Qt::LinksAccessibleByMouse);

     QMainWindow::keyReleaseEvent(k);
}

void Note::setSearchBarText(QString str)
{
    searchBar->setText(str);
    searchBar->selectNextExpression();
    searchBar->setVisible(true);
}

void Note::showContextMenu(const QPoint &pt)
{
     if(searchBar->isVisible() || toolbar->isVisible())
       showHideToolbars->setText(tr("Hide Toolbars"));
     else
       showHideToolbars->setText(tr("Show Toolbars"));

     QMenu *menu = textBrowser->createStandardContextMenu();
     menu->addAction(showHideToolbars);
     menu->exec(textBrowser->mapToGlobal(pt));
}

void Note::showOrHideToolbars()
{
     if(searchBar->isVisible() || toolbar->isVisible())
     {
          if(searchBar->isVisible())
            searchBar->setVisible(false);
          if(toolbar->isVisible())
            toolbar->setVisible(false);
     }
     else
     {
          searchBar->setVisible(true);
          toolbar->setVisible(true);
     }
}

void Note::showAfterLoaded()
{
    showAfterLoading_ = true;
}
