#ifndef NOTE_H
#define NOTE_H

#include "ui_note.h"
#include <QTimer>
#include <QFontComboBox>
#include <QComboBox>
#include <QKeyEvent>
#include <QDateTime>
#include <QTextEdit>
#include "textdocument.h"

class TextFormattingToolbar;
class TextSearchToolbar;
class NewNoteName;
class TextEdit;
class Highlighter;
class NoteDescriptor;

class Note : public QMainWindow, public Ui::Note {
     Q_OBJECT // important for creating own singals and slots
 
     public:
      Note(QString filePath, QWidget *parent = 0);
      ~Note();
      QString text, journalsPath, notePath;
      bool    searchbarVisible;

     private:
      TextEdit *textEdit;
      TextDocument *textDocument;

      QMenu     *menu;

      TextFormattingToolbar * toolbar;
      TextSearchToolbar *searchB;
      bool      alreadyAsked;
      NewNoteName *newNote;
      Highlighter *highlighter;

      NoteDescriptor * noteDescriptor_;



//      QTimer    *jTimer;
//      QDateTime noteModified, journalModified;
//      QTimer  *timer;

     public slots:
//      void saveAll();
//      void load();
//      void reload();
//      void save_or_not();
//      void resetAll();
//      void askForName();
//      void getNewName();
      void selectNextExpression();
      void selectPreviousExpression();
      void highlightText(QString str);

     protected:
      void keyPressEvent(QKeyEvent *k);
      virtual void showEvent(QShowEvent* show_Note);
      virtual void closeEvent(QCloseEvent* close_Note);
};

#endif
