#ifndef NOTE_H
#define NOTE_H

#include "ui_note.h"
#include <QTimer>

class Note : public QDialog, public Ui::Note {
     Q_OBJECT // important for creating own singals and slots
 
     public:
      Note(QWidget *parent = 0);
      ~Note();
      QString text, notesPath;

     private:
      QTimer *timer;

     public slots:
      void saveNote();
      void dontSave();
      QTextCharFormat getFormatOnWordOrSelection();
      void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
      void boldText();
      void italicText();
      void underlinedText();
      void getFontAndPointSizeOfText();
      void fontOfText();
      void pointSizeOfText();

     signals:

     protected:
      virtual void showEvent(QShowEvent* show_Note);
      virtual void closeEvent(QCloseEvent* close_Note);
};

#endif
