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

#ifndef TEXTFORMATTINGTOOLBAR_H
#define TEXTFORMATTINGTOOLBAR_H

#include <QToolBar>
#include <QFontComboBox>
#include <QTextEdit>

/**
  * a toolbar with buttons to format text on a QTextEdit
  * you must call addToolBar on the MainWindow to add this toolbar
  *
  */

class TextFormattingToolbar : public QToolBar
{
    Q_OBJECT
public:
    explicit TextFormattingToolbar(QTextEdit* textEdit,QWidget *parent = 0);
    
public slots:

    void getFontAndPointSizeOfText(const QTextCharFormat &format);
    void setFont(QFont font);

private slots:
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void clearCharFormat(); // sets a default QTextCharFormat()
    //void removeWhitespace(); // not connected to a toolbutton
    void boldText();
    void italicText();
    void underlinedText();
    void strikedOutText();
    void insertBulletPoints(); // insert bullet points  •
    void coloredText();
    void markedText();
    void insertHyperlink();
    void fontOfText(const QString &f);
    void pointSizeOfText(const QString &p);

    void updateBulletPointToolbarButton();
private:
    QFontComboBox *fontComboBox;
    QComboBox *fontSizeComboBox;
    QAction   *actionTextBold;
    QAction *actionTextItalic;
    QAction *actionTextUnderline;
    QAction *actionTextColor;
    QAction *actionTextBColor;
    QAction *actionTextStrikeOut;
    QAction *actionInsertHyperlink;
    QAction *actionClearFormatting;
    QAction *actionRemoveWhitespace;
    QAction *actionBulletPoint;
    QTextEdit * textEdit_;
};

#endif // TEXTFORMATTINGTOOLBAR_H
