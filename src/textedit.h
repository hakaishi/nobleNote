#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QTextEdit>

/**
  * a text edit with a custom focus in event
  *
  */

class TextEdit : public QTextEdit{
     Q_OBJECT

     public:
      TextEdit(QWidget *parent = 0);

     signals:
      void signalFocusInEvent();
      void signalFocusOutEvent();

     protected:
      virtual void focusInEvent(QFocusEvent *fe);
      virtual void focusOutEvent(QFocusEvent *e);
};

#endif // TEXTEDIT_H

