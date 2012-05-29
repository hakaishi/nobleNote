#include "textedit.h"
#include <QTextEdit>

TextEdit::TextEdit(QWidget *parent) : QTextEdit(parent){ }

void TextEdit::focusInEvent(QFocusEvent *event){
     emit signalFocusInEvent();
     QTextEdit::focusInEvent(event);
}

void TextEdit::focusOutEvent(QFocusEvent *e)
{
    emit signalFocusOutEvent();
    QTextEdit::focusOutEvent(e);
}
