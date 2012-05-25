#ifndef TEXTSEARCHTOOLBAR_H
#define TEXTSEARCHTOOLBAR_H

#include <QToolBar>
#include <QCheckBox>
#include <QPushButton>
#include <QToolButton>
#include <QTextEdit>
#include <QLineEdit>

class TextSearchToolbar : public QToolBar
{
    Q_OBJECT
public:
     explicit TextSearchToolbar(QTextEdit* textEdit,QWidget *parent = 0);
     QLineEdit    *searchLine;
     QPushButton *findNext, *findPrevious;
     QToolButton *closeSearch;
     QCheckBox   *caseSensitiveBox;  
     QTextEdit   *textEdit_;  
};

#endif // TEXTSEARCHTOOLBAR_H
