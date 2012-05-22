#ifndef SEARCHBOX_H
#define SEARCHBOX_H

#include "ui_searchbox.h"

class Highlighter;

class SearchBox : public QDialog, public Ui::SearchBox {
     Q_OBJECT
 
     public:
      SearchBox(QWidget *parent = 0);
      ~SearchBox();
      QTextDocument *doc;

     private:
      Highlighter *highlighter;

     private slots:
      void searchAndMark();
};

#endif //SEARCHBOX_H
