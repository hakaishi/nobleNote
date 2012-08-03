#ifndef TEXTBROWSER_H
#define TEXTBROWSER_H

#include <QTextBrowser>

/**
  * a text edit with a custom focus in event
  *
  */

class TextBrowser : public QTextBrowser{
     Q_OBJECT

     public:
      TextBrowser(QWidget *parent = 0);

     signals:
      void signalFocusInEvent();
      void signalFocusOutEvent();

public slots:
      void slotSetReadOnly(bool ro); // wrapper slot

     protected:
      virtual void focusInEvent(QFocusEvent *fe);
      virtual void focusOutEvent(QFocusEvent *e);

private slots:
      void openLinkInBrowser(const QUrl link);
};

#endif // TEXTEDIT_H

