#ifndef NEWNOTENAME_H
#define NEWNOTENAME_H

#include "ui_newnotename.h"

class NewNoteName : public QDialog, public Ui::NewNoteName {
     Q_OBJECT
 
     public:
      NewNoteName(QWidget *parent = 0);
      ~NewNoteName();

     signals:
      void newName();
};

#endif //NewNoteName
