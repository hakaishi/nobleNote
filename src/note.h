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

#ifndef NOTE_H
#define NOTE_H

#include "ui_note.h"
#include "textdocument.h"
#include <QTimer>
#include <QKeyEvent>
#include <QDateTime>
#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QMenu>
#include <QFuture>
#include "textbrowser.h"

class TextFormattingToolbar;
class TextSearchToolbar;
class Highlighter;
class NoteDescriptor;

/**
 * @brief The Note class is the widget that displays note content
 *
 * most lifecycle handling and loading/saving of html and xml type notes is handled
 * in NoteDescriptor.
 *
 * note loading workflow:
 * //TODO workflow should be refactored to load the file before creating the note window
 *
 * this clas creates a NoteDescriptor
 * the NoteDescriptor loads the html file asynchronously and reads the uuid
 * a signal onLoadFinished() is sent, this is used in this class to call loadSizeAndShow
 * loadSizeAndShow reads the UUId from NoteDescriptor, that is now initialized
 * (before, it is 0000-...) and loads the Note Window size from the QSettings
 * loadSizeAndSHow resizes the Note window and calls show()
 *
 * you should not directly call show() but instead call showAfterLoaded()
 * to set the flag to show the Note window after all initialization has been finished
 */

class Note : public QMainWindow, public Ui::Note {
     Q_OBJECT // important for creating own singals and slots
 
     public:
      Note(QString filePath, QWidget *parent = 0);
      NoteDescriptor* noteDescriptor() const { return noteDescriptor_; }

      void highlightText(const QString & str);

      QTextEdit * textEdit() const { return textBrowser;}

      QFuture<void> future() const { return future_;}

     private:
      TextBrowser *textBrowser;
      TextDocument *textDocument;
      QMenu    *menu;
      QAction  *showHideToolbars;

      TextFormattingToolbar *toolbar;
      TextSearchToolbar *searchBar;
      QFuture<void> future_; // a future that holds the worker thread that is invoked when window states are changed

      NoteDescriptor *noteDescriptor_;
      bool showAfterLoading_; // state variable, call show() after loadSizeAndShow()

      // this is called asynchronously when the window is closed and saves geometry etc.
      void saveWindowState(QVariantList variantList);
public slots:
      void setSearchBarText(QString str);

      void showAfterLoaded(); // calls show() after loading data and size settings

     private slots:
      void showContextMenu(const QPoint &pt);
      void showOrHideToolbars();

     protected:
      void keyPressEvent(QKeyEvent *k);
      void keyReleaseEvent(QKeyEvent *k);
      virtual void closeEvent(QCloseEvent *close_Note);
      // focus events must be overridden in the TextBrowser class

private slots:
      void loadSizeAndShow(); // load size from settings and resize
};

#endif
