#ifndef NOTE_H
#define NOTE_H

#include "ui_note.h"
#include <QTimer>
#include <QFontComboBox>
#include <QComboBox>

class Note : public QMainWindow, public Ui::Note {
     Q_OBJECT // important for creating own singals and slots
 
     public:
      Note(QWidget *parent = 0);
      ~Note();
      QString text, journalsPath, notesPath;

     private:
      QTimer *timer;
      QMenu *menu;
      QFontComboBox *fontComboBox;
      QComboBox *comboSize;
      QAction *actionTextBold, *actionTextItalic,
              *actionTextUnderline, *actionTextColor, *actionTextBColor;

     public slots:
      void saveJournal();
      void saveNote();
      void dontSave();
      void setupTextFormattingOptions();
      QTextCharFormat getFormatOnWordOrSelection();
      void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
      void boldText();
      void italicText();
      void underlinedText();
      void coloredText();
      void markedText();
      void getFontAndPointSizeOfText(const QTextCharFormat &format);
      void fontOfText(const QString &f);
      void pointSizeOfText(const QString &f);

     signals:

     protected:
      virtual void showEvent(QShowEvent* show_Note);
      virtual void closeEvent(QCloseEvent* close_Note);
};

#endif
