#ifndef NOTE_H
#define NOTE_H

#include "ui_note.h"
#include <QTimer>
#include <QFontComboBox>
#include <QComboBox>
#include <QKeyEvent>
#include <QDateTime>

class TextFormattingToolbar;
class TextSearchToolbar;
class NewNoteName;
class TextEdit;
class Highlighter;

class Note : public QMainWindow, public Ui::Note {
     Q_OBJECT // important for creating own singals and slots
 
     public:
      Note(QWidget *parent = 0);
      ~Note();
      QString text, journalsPath, notesPath;
      QTimer  *timer;

     private:
      TextEdit  *textEdit;
      QTimer    *jTimer;
      QMenu     *menu;
      QDateTime noteModified, journalModified;
      TextFormattingToolbar * toolbar;
      TextSearchToolbar *searchB;
      bool      alreadyAsked;
      NewNoteName *newNote;
      Highlighter *highlighter;

     public slots:
      void saveAll();
      void load();
      void reload();
      void save_or_not();
      void resetAll();
      void askForName();
      void getNewName();
      void selectNextExpression();
      void selectPreviousExpression();
      void highlightText();

     signals:
      void closing(QObject *object);

     protected:
      void keyPressEvent(QKeyEvent *k);
      virtual void showEvent(QShowEvent* show_Note);
};

#endif
