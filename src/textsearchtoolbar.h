#ifndef TEXTSEARCHTOOLBAR_H
#define TEXTSEARCHTOOLBAR_H

#include <QToolBar>
#include <QCheckBox>
#include <QToolButton>
#include <QTextEdit>
#include <QLineEdit>

class LineEdit;

class TextSearchToolbar : public QToolBar
{
    Q_OBJECT
public:
     explicit TextSearchToolbar(QTextEdit* textEdit,QWidget *parent = 0);
     QLineEdit   *searchLine;
     QToolButton *findNext, *findPrevious, *closeSearch;
     QCheckBox   *caseSensitiveBox;  
     QTextEdit   *textEdit_;  
};

#endif // TEXTSEARCHTOOLBAR_H
