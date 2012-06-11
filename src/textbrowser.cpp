#include "textbrowser.h"
#include <QTextBrowser>

TextBrowser::TextBrowser(QWidget *parent) : QTextBrowser(parent){ }

void TextBrowser::focusInEvent(QFocusEvent *event){
     emit signalFocusInEvent();
     QTextEdit::focusInEvent(event);
}

void TextBrowser::focusOutEvent(QFocusEvent *e)
{
    emit signalFocusOutEvent();
    QTextEdit::focusOutEvent(e);
}


void TextBrowser::slotSetReadOnly(bool ro)
{
    this->setReadOnly(ro);
}
