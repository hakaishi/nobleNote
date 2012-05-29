#include "textdocument.h"
#include <QTimer>

TextDocument::TextDocument(QObject *parent) :
    QTextDocument(parent)
{
    timer = new QTimer(this);
    delay_ = 2000;
    timer->setInterval(delay_);
    timer->setSingleShot(true);
    connect(this,SIGNAL(modificationChanged(bool)),this,SLOT(startStopTimer(bool)));
    connect(timer,SIGNAL(timeout()),this,SLOT(sendDelayedModificationChanged()));
}



void TextDocument::startStopTimer(bool b)
{
    if(b)
        timer->start();
    else
        timer->stop();
}

void TextDocument::sendDelayedModificationChanged()
{
    if(this->isModified())
    {
        emit delayedModificationChanged(true);
    }
}
