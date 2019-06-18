/* nobleNote, a note taking application
 * Copyright (C) 2019 Christian Metscher <hakaishi@web.de>,
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

#include "textsearchtoolbar.h"
#include "lineedit.h"
#include "highlighter.h"

#include <QTimer>

TextSearchToolbar::TextSearchToolbar(QTextEdit * textEdit, QWidget *parent) :
       QToolBar(parent), textEdit_(textEdit){

     setWindowTitle(tr("Search bar"));
     setObjectName(tr("Searchtoolbar"));

     typingTimer = new QTimer(this),
     typingTimer->setSingleShot(true);
     int typingThrottleTime = textEdit_->document()->characterCount() > 5000 ? 300 : 0;
     typingTimer->setInterval(typingThrottleTime);
     connect(typingTimer,&QTimer::timeout,this,&TextSearchToolbar::selectNextExpression);

     closeSearch = new QToolButton(this);
     closeSearch->setText("X");
     closeSearch->setShortcut(Qt::Key_Escape);
     addWidget(closeSearch);

     searchLine_ = new LineEdit(this);
     searchLine_->setPlaceholderText(tr("Enter search argument"));
     addWidget(searchLine_);

     findPrevious = new QToolButton(this);
     findPrevious->setText(tr("Find &previous"));
     addWidget(findPrevious);

     findNext = new QToolButton(this);
     findNext->setText(tr("Find &next"));
     addWidget(findNext);

     caseSensitiveBox = new QCheckBox(this);
     caseSensitiveBox->setText(tr("&Case sensitive"));
     addWidget(caseSensitiveBox);


     connect(closeSearch, SIGNAL(clicked(bool)), this, SLOT(hide()));

     connect(searchLine_, &QLineEdit::textChanged, this,
             [this](){
         typingTimer->start();
     });


     connect(findNext, SIGNAL(clicked(bool)), SLOT(selectNextExpression()));
     connect(findPrevious, SIGNAL(clicked(bool)), SLOT(selectPreviousExpression()));
     connect(searchLine_, SIGNAL(returnPressed()), SLOT(selectNextExpression()));
}

void TextSearchToolbar::selectNextExpression(){
     if(this->caseSensitiveBox->isChecked()){
       if(!textEdit_->find(this->searchLine_->text(), QTextDocument::FindCaseSensitively)){
         textEdit_->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
         textEdit_->find(this->searchLine_->text(), QTextDocument::FindCaseSensitively);
       }
     }
     else{
       if(!textEdit_->find(this->searchLine_->text())){
         textEdit_->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
         textEdit_->find(this->searchLine_->text());
       }
     }
     highlightText(this->searchLine_->text());
}

void TextSearchToolbar::setText(const QString &text)
{
    searchLine_->setText(text);
    highlightText(textEdit_->textCursor().selectedText());
}

void TextSearchToolbar::selectPreviousExpression(){
     if(this->caseSensitiveBox->isChecked()){
       if(!textEdit_->find(this->searchLine_->text(), QTextDocument::FindCaseSensitively |
           QTextDocument::FindBackward)){
         textEdit_->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
         textEdit_->find(this->searchLine_->text(), QTextDocument::FindCaseSensitively |
           QTextDocument::FindBackward);
       }
     }
     else{
       if(!textEdit_->find(this->searchLine_->text(), QTextDocument::FindBackward)){
         textEdit_->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
         textEdit_->find(this->searchLine_->text(), QTextDocument::FindBackward);
       }
     }
     highlightText(this->searchLine_->text());
}

void TextSearchToolbar::highlightText(QString str){
     highlighter = new Highlighter(textEdit_->document());
     highlighter->expression = str;
     if(this->caseSensitiveBox->isChecked())
       highlighter->caseSensitive = true;
     else
       highlighter->caseSensitive = false;
     connect(closeSearch, SIGNAL(clicked(bool)), highlighter, SLOT(deleteLater()));
     connect(closeSearch, SIGNAL(clicked(bool)), textEdit_, SLOT(setFocus()));
     connect(closeSearch, SIGNAL(clicked(bool)), this->searchLine_, SLOT(clear()));

     textEdit_->ensureCursorVisible();
}
