#include "textsearchtoolbar.h"

TextSearchToolbar::TextSearchToolbar(QTextEdit * textEdit, QWidget *parent) :
       QToolBar(parent), textEdit_(textEdit){

     setWindowTitle(tr("Search bar"));
     setVisible(false);    

     searchLine = new QLineEdit(this);
     searchLine->setPlaceholderText(tr("Enter search argument"));
     addWidget(searchLine);

     caseSensitiveBox = new QCheckBox(this);
     caseSensitiveBox->setText(tr("&Case sensitive"));
     addWidget(caseSensitiveBox);

     findNext = new QPushButton(this);
     findNext->setText(tr("Find &next"));
     addWidget(findNext);

     findPrevious = new QPushButton(this);
     findPrevious->setText(tr("Find &previous"));
     addWidget(findPrevious);

     closeSearch = new QToolButton(this);
     closeSearch->setText("X");
     addWidget(closeSearch);


     connect(closeSearch, SIGNAL(clicked(bool)), this, SLOT(hide()));
}
