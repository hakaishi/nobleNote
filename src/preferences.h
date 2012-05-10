#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "ui_preferences.h"

class Preferences : public QDialog, public Ui::Preferences {
     Q_OBJECT
 
     public:
      Preferences(QWidget *parent = 0);
      ~Preferences();

     private slots:
      void saveSettings();

     signals:
      void sendPathChanged();
};

#endif //PREFERENCES_H
