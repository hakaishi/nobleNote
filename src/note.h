#ifndef NOTE_H
#define NOTE_H

#include "ui_note.h"
#include <QTimer>
#include <QFontComboBox>
#include <QComboBox>
#include <QKeyEvent>
#include <QDateTime>
#include "textformattingtoolbar.h"

class Note : public QMainWindow, public Ui::Note {
     Q_OBJECT // important for creating own singals and slots
 
     public:
      Note(QWidget *parent = 0);
      ~Note();
      QString text, journalsPath, notesPath;
      QTimer  *timer;

     private:
      QTimer    *jTimer;
      QMenu     *menu;
      QDateTime noteModified, journalModified;
      TextFormattingToolbar * toolbar;

     public slots:
      void saveAll();
      void load();
      void save_or_not();
      void resetAll();


     signals:
      void closing(QObject *object);

     protected:
      void keyPressEvent(QKeyEvent *k);
      virtual void showEvent(QShowEvent* show_Note);
      virtual void focusInEvent(QFocusEvent *fe);
};

#endif
