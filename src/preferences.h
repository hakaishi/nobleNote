#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "ui_preferences.h"
#include <QSettings>

class Preferences : public QDialog, public Ui::Preferences {
     Q_OBJECT
 
     public:
      Preferences(QWidget *parent = 0);
      ~Preferences();
      QString rootPath;

     private:
      QSettings *settings;

     private slots:
      void saveSettings();
      void openDir();

     signals:
      void sendPathChanged();
};

#endif //PREFERENCES_H
