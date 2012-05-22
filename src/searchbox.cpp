#include "searchbox.h"
#include "highlighter.h"

SearchBox::SearchBox(QWidget *parent): QDialog(parent){
     setupUi(this);

     setAttribute(Qt::WA_DeleteOnClose);

     searchEdit->setFocus();

     connect(findButton, SIGNAL(clicked(bool)), this, SLOT(searchAndMark()));
}

SearchBox::~SearchBox(){}

void SearchBox::searchAndMark(){
     highlighter = new Highlighter(doc);
     highlighter->expression = searchEdit->text();
}
