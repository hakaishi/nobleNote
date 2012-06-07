#ifndef WELCOME_H
#define WELCOME_H

#include "ui_welcome.h"

class LineEdit;

class Welcome : public QDialog, public Ui::Welcome{
     Q_OBJECT

     public:
      Welcome(QWidget *parent = 0);

     private:
      LineEdit *path;

     private slots:
      void openDir();
      void setRootDir();
};

#endif // WELCOME_H
