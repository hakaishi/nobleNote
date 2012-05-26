#include "textsearchtoolbar.h"

TextSearchToolbar::TextSearchToolbar(QTextEdit * textEdit, QWidget *parent) :
       QToolBar(parent), textEdit_(textEdit){

     setWindowTitle(tr("Search bar"));
     setVisible(false);    

     closeSearch = new QToolButton(this);
     closeSearch->setText("X");
     addWidget(closeSearch);

     searchLine = new LineEdit(this);
     searchLine->setPlaceholderText(tr("Enter search argument"));
     addWidget(searchLine);

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
}
