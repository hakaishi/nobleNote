#include "newnotename.h"

NewNoteName::NewNoteName(QWidget *parent): QDialog(parent){
     setupUi(this);

     connect(this, SIGNAL(accepted()), this, SIGNAL(newName()));
}

NewNoteName::~NewNoteName(){}
