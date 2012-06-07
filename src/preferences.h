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
      QString    originalRootPath;

     private slots:
      void saveSettings();
      void openDir();

     signals:
      void sendPathChanged();

     protected:
      virtual void showEvent(QShowEvent* show_pref);
};

#endif //PREFERENCES_H
