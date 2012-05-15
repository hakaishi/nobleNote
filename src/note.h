#ifndef NOTE_H
#define NOTE_H

#include "ui_note.h"
#include <QTimer>
#include <QFontComboBox>
#include <QComboBox>
#include <QKeyEvent>

class Note : public QMainWindow, public Ui::Note {
     Q_OBJECT // important for creating own singals and slots
 
     public:
      Note(QWidget *parent = 0);
      ~Note();
      QString text, journalsPath, notesPath;
      QTimer *timer;

     private:
      QTimer *jTimer;
      QMenu *menu;
      QFontComboBox *fontComboBox;
      QComboBox *comboSize;
      QAction *actionTextBold, *actionTextItalic,
              *actionTextUnderline, *actionTextColor,
      *actionTextBColor, *actionTextStrikeOut;

     public slots:
      void saveText();
      void resetAll();
      void setupTextFormattingOptions();
      void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
      void boldText();
      void italicText();
      void underlinedText();
      void strikedOutText();
      void coloredText();
      void markedText();
      void getFontAndPointSizeOfText(const QTextCharFormat &format);
      void fontOfText(const QString &f);
      void pointSizeOfText(const QString &f);

     signals:
      void closing(QString &path);

     protected:
      void keyPressEvent(QKeyEvent *k);
      virtual void showEvent(QShowEvent* show_Note);
};

#endif
