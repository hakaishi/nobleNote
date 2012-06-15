#ifndef NOTE_H
#define NOTE_H

#include "ui_note.h"
#include "textdocument.h"
#include <QTimer>
#include <QFontComboBox>
#include <QComboBox>
#include <QKeyEvent>
#include <QDateTime>
#include <QTextBrowser>
#include <QDialogButtonBox>

class TextFormattingToolbar;
class TextSearchToolbar;
class NewNoteName;
class TextBrowser;
class Highlighter;
class NoteDescriptor;

class Note : public QMainWindow, public Ui::Note {
     Q_OBJECT // important for creating own singals and slots
 
     public:
      Note(QString filePath, QWidget *parent = 0);
      ~Note();
      NoteDescriptor* noteDescriptor() const { return noteDescriptor_; }
      bool    searchbarVisible;

     private:
      QDialogButtonBox *buttonBox;
      TextBrowser *textBrowser;
      TextDocument *textDocument;

      QMenu     *menu;

      TextFormattingToolbar * toolbar;
      TextSearchToolbar *searchB;
      bool      alreadyAsked;
      NewNoteName *newNote;
      Highlighter *highlighter;

      NoteDescriptor * noteDescriptor_;

     public slots:
      void undoAll();
      void selectNextExpression();
      void selectPreviousExpression();
      void highlightText(QString str);
      void setSearchBarText(QString str);

     protected:
      void keyPressEvent(QKeyEvent *k);
      void keyReleaseEvent(QKeyEvent *k);
      virtual void showEvent(QShowEvent* show_Note);
      virtual void closeEvent(QCloseEvent* close_Note);
};

#endif
