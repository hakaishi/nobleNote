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

#ifndef NOTE_H
#define NOTE_H

#include "ui_note.h"
#include "textdocument.h"
#include <QTimer>
#include <QKeyEvent>
#include <QDateTime>
#include <QTextBrowser>
#include <QDialogButtonBox>
#include "textbrowser.h"

class TextFormattingToolbar;
class TextSearchToolbar;
class Highlighter;
class NoteDescriptor;

class Note : public QMainWindow, public Ui::Note {
     Q_OBJECT // important for creating own singals and slots
 
     public:
      Note(QString filePath, QWidget *parent = 0);
      NoteDescriptor* noteDescriptor() const { return noteDescriptor_; }

      void highlightText(const QString & str);
      bool searchbarVisible;

      QTextEdit * textEdit() const { return textBrowser;}

     private:
      TextBrowser *textBrowser;
      TextDocument *textDocument;

      TextFormattingToolbar *toolbar;
      TextSearchToolbar *searchBar;

      NoteDescriptor *noteDescriptor_;

     public slots:
      void setSearchBarText(QString str);

     protected:
      void keyPressEvent(QKeyEvent *k);
      void keyReleaseEvent(QKeyEvent *k);
      virtual void showEvent(QShowEvent* event);
      virtual void closeEvent(QCloseEvent *close_Note);
};

#endif
