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

#include "textformattingtoolbar.h"
#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QInputDialog>
#include <QTextBlock>
#include <QDebug>
#include <QTextDocumentFragment>

TextFormattingToolbar::TextFormattingToolbar(QTextEdit * textEdit, QWidget *parent) :
    QToolBar(parent), textEdit_(textEdit)
{
    setWindowTitle(tr("Format actions"));
    setObjectName(tr("Formattoolbar"));

    actionTextBold = new QAction(QIcon::fromTheme("format-text-bold",
      QIcon(":bold")), tr("&Bold"), this);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    connect(actionTextBold, SIGNAL(triggered()), this, SLOT(boldText()));
    addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    actionTextItalic = new QAction(QIcon::fromTheme("format-text-italic",
      QIcon(":italic")), tr("&Italic"), this);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    connect(actionTextItalic, SIGNAL(triggered()), this, SLOT(italicText()));
    addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    actionTextUnderline = new QAction(QIcon::fromTheme("format-text-underline",
      QIcon(":underlined")), tr("&Underline"), this);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    connect(actionTextUnderline, SIGNAL(triggered()), this, SLOT(underlinedText()));
    addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    actionTextStrikeOut = new QAction(QIcon::fromTheme("format-text-strikethrough",
      QIcon(":strikedout")), tr("&Strike Out"), this);
    actionTextStrikeOut->setShortcut(Qt::CTRL + Qt::Key_S);
    actionTextStrikeOut->setPriority(QAction::LowPriority);
    QFont strikeOut;
    strikeOut.setStrikeOut(true);
    actionTextStrikeOut->setFont(strikeOut);
    connect(actionTextStrikeOut, SIGNAL(triggered()), this, SLOT(strikedOutText()));
    addAction(actionTextStrikeOut);
    actionTextStrikeOut->setCheckable(true);

    actionInsertHyperlink = new QAction(QIcon::fromTheme("emblem-web",QIcon(":hyperlink")),tr("&Hyperlink"),this);
    actionInsertHyperlink->setShortcut(Qt::CTRL + Qt::Key_K); // word shortcut
    actionInsertHyperlink->setPriority(QAction::LowPriority);
    connect(actionInsertHyperlink,SIGNAL(triggered()),this,SLOT(insertHyperlink()));
    addAction(actionInsertHyperlink);

    actionClearFormatting = new QAction(QIcon::fromTheme("TODO",QIcon(":clearFormatting")),tr("&Clear formatting"),this);
    actionClearFormatting->setPriority(QAction::LowPriority);
    actionClearFormatting->setShortcut(Qt::CTRL + Qt::Key_Space); // ms word clear font formatting shortcut
    connect(actionClearFormatting,SIGNAL(triggered()),this,SLOT(clearCharFormat()));
    addAction(actionClearFormatting);

    actionRemoveWhitespace = new QAction(/*QIcon::fromTheme("TODO",QIcon("")),tr("&Remove Whitespace"),*/this);
    actionRemoveWhitespace->setPriority(QAction::LowPriority);
    connect(actionRemoveWhitespace,SIGNAL(triggered()),this,SLOT(removeWhitespace()));
    addAction(actionRemoveWhitespace);

    QPixmap textPix(16, 16);
    textPix.fill(textEdit_->palette().windowText().color());
    actionTextColor = new QAction(textPix, tr("&Text color..."), this);
    connect(actionTextColor, SIGNAL(triggered()), this, SLOT(coloredText()));
    addAction(actionTextColor);

    QPixmap bPix(16, 16);
    bPix.fill(textEdit_->palette().base().color());
    actionTextBColor = new QAction(bPix, tr("&Background color..."), this);
    connect(actionTextBColor, SIGNAL(triggered()), this, SLOT(markedText()));
    addAction(actionTextBColor);

    fontComboBox = new QFontComboBox(this);
    fontComboBox->setFocusPolicy(Qt::TabFocus);
    addWidget(fontComboBox);
    connect(fontComboBox, SIGNAL(activated(QString)), this, SLOT(fontOfText(QString)));

    fontSizeComboBox = new QComboBox(this);
    fontSizeComboBox->setFocusPolicy(Qt::TabFocus);
    fontSizeComboBox->setObjectName("comboSize");
    addWidget(fontSizeComboBox);
    fontSizeComboBox->setEditable(true);
    connect(fontSizeComboBox, SIGNAL(activated(QString)), this, SLOT(pointSizeOfText(QString)));

    QFontDatabase db;
    foreach(int size, db.standardSizes())
      fontSizeComboBox->addItem(QString::number(size));

    connect(textEdit_, SIGNAL(currentCharFormatChanged(QTextCharFormat)), this,
      SLOT(getFontAndPointSizeOfText(QTextCharFormat)));
}

void TextFormattingToolbar::mergeFormatOnWordOrSelection(const QTextCharFormat &format){
     QTextCursor cursor = textEdit_->textCursor();
     if(cursor.selectedText() == 0)
       cursor.select(QTextCursor::WordUnderCursor);
     cursor.setCharFormat(format);
     textEdit_->mergeCurrentCharFormat(format);
}

void TextFormattingToolbar::clearCharFormat()
{
     QTextCursor cursor = textEdit_->textCursor();
     if(cursor.selectedText() == 0)
       cursor.select(QTextCursor::WordUnderCursor);
     cursor.setCharFormat(QTextCharFormat());
     textEdit_->setCurrentCharFormat(QTextCharFormat());
}

void TextFormattingToolbar::removeWhitespace()
{
    textEdit_->setPlainText(textEdit_->toPlainText().replace(" ", ""));
}

void TextFormattingToolbar::getFontAndPointSizeOfText(const QTextCharFormat &format){
     QFont f = format.font();
     fontComboBox->setCurrentIndex(fontComboBox->findText(QFontInfo(f).family()));
     fontSizeComboBox->setCurrentIndex(fontSizeComboBox->findText(QString::number(f.pointSize())));
     actionTextBold->setChecked(f.bold());
     actionTextItalic->setChecked(f.italic());
     actionTextUnderline->setChecked(f.underline());
     actionTextStrikeOut->setChecked(f.strikeOut());
     QPixmap textPix(16,16);
     if(format.foreground().style() == Qt::NoBrush)
       textPix.fill(textEdit_->palette().windowText().color());
     else
       textPix.fill(format.foreground().color());
     actionTextColor->setIcon(textPix);
     QPixmap bPix(16,16);
     if(format.background().style() == Qt::NoBrush)
       bPix.fill(textEdit_->palette().base().color());
     else
       bPix.fill(format.background().color());
     actionTextBColor->setIcon(bPix);
}

void TextFormattingToolbar::boldText(){
     QTextCharFormat fmt;
     fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
     mergeFormatOnWordOrSelection(fmt);
}

void TextFormattingToolbar::italicText(){
     QTextCharFormat fmt;
     fmt.setFontItalic(actionTextItalic->isChecked());
     mergeFormatOnWordOrSelection(fmt);
}

void TextFormattingToolbar::underlinedText(){
     QTextCharFormat fmt;
     fmt.setFontUnderline(actionTextUnderline->isChecked());
     mergeFormatOnWordOrSelection(fmt);
}

void TextFormattingToolbar::strikedOutText(){
    QTextCharFormat fmt;
    fmt.setFontStrikeOut(actionTextStrikeOut->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextFormattingToolbar::coloredText(){
     QColor col = QColorDialog::getColor(textEdit_->textColor(), this);
     if(!col.isValid())
       return;
     QTextCharFormat fmt;
     fmt.setForeground(QBrush(col,Qt::SolidPattern));
     mergeFormatOnWordOrSelection(fmt);
     QPixmap pix(16, 16);
     pix.fill(col);
     actionTextColor->setIcon(pix);
}

void TextFormattingToolbar::markedText(){
     QColor col = QColorDialog::getColor(textEdit_->textBackgroundColor(), this);
     if(!col.isValid())
       return;
     QTextCharFormat fmt;
     fmt.setBackground(QBrush(col,Qt::SolidPattern));
     mergeFormatOnWordOrSelection(fmt);
     QPixmap pix(16, 16);
     pix.fill(col);
     actionTextBColor->setIcon(pix);
}

void TextFormattingToolbar::insertHyperlink()
{
    QTextCursor cursor = textEdit_->textCursor();

    QString selectedText = cursor.selectedText();

    bool ok;
    QString link = QInputDialog::getText(textEdit_,tr("Insert hyperlink"),tr("Addr&ess:"),QLineEdit::Normal,selectedText,&ok);
    if(!ok)
        return;
/*TODO: de-formatting
    if(link.isEmpty){
      resetFonts()//or something like that
      return;
    }*/

/*TODO: check if link/e-mail is valid with the following regexps
QRegExp(">\\b((((https?|ftp)://)|(www\\.))[a-zA-Z0-9_\\.\\-\\?]+)\\b(<?)" , Qt::CaseInsensitive)
QRegExp(">\\b([a-zA-Z0-9_\\.\\-]+@[a-zA-Z0-9_\\.\\-]+)\\b(<?)", Qt::CaseInsensitive)
*/
    if(selectedText.isEmpty())
        selectedText = link;

    if(link.contains("@"))
      cursor.insertHtml("<a href=mailto:"+link+"\">"+selectedText+"</a>");
    else
      cursor.insertHtml("<a href=\""+link+"\">"+selectedText+"</a>");
}

void TextFormattingToolbar::fontOfText(const QString &f){
     QTextCharFormat fmt;
     fmt.setFontFamily(f);
     mergeFormatOnWordOrSelection(fmt);
}

void TextFormattingToolbar::pointSizeOfText(const QString &p){
     qreal pointSize = p.toFloat();
     if(p.toFloat() > 0){
       QTextCharFormat fmt;
       fmt.setFontPointSize(pointSize);
       mergeFormatOnWordOrSelection(fmt);
     }
}

void TextFormattingToolbar::setFont(QFont font)
{
     fontComboBox->setCurrentFont(font);
     fontSizeComboBox->setCurrentIndex(fontSizeComboBox->findText(QString::number(
       font.pointSize())));
}
