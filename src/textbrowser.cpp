#include "textbrowser.h"
#include <QTextBrowser>
#include <QDesktopServices>

TextBrowser::TextBrowser(QWidget *parent) : QTextBrowser(parent)
{
    this->viewport()->setCursor(Qt::IBeamCursor);
    setOpenLinks(false); // also disables external links
    connect(this,SIGNAL(anchorClicked(QUrl)),this,SLOT(openLinkInBrowser(QUrl))); // because all links should be opened in the web browser
}

void TextBrowser::focusInEvent(QFocusEvent *event){
     emit signalFocusInEvent();
     QTextEdit::focusInEvent(event);
}

void TextBrowser::focusOutEvent(QFocusEvent *e)
{
    emit signalFocusOutEvent();
    QTextEdit::focusOutEvent(e);
}

void TextBrowser::openLinkInBrowser(const QUrl link)
{
    QDesktopServices::openUrl(link);
}


void TextBrowser::slotSetReadOnly(bool ro)
{
    this->setReadOnly(ro);
}
