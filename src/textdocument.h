#ifndef TEXTDOCUMENT_H
#define TEXTDOCUMENT_H

#include <QTextDocument>

/**
 * re-emits the modificationChanged signal of a QTextDocument after a timespan of delay() seconds
 * if the modified property is set to false elswhere in that timespan, the signal is not emitted
 * this class does not modify the modified property of the QTextDocument
 *
 **/

class TextDocument : public QTextDocument
{
    Q_OBJECT
public:
    explicit TextDocument(QObject *parent = 0);
    void setDelay(int delay){ delay_ = delay; }
    int delay() const { return delay_; }
    
signals:

    void delayedModificationChanged();
    
private slots:

    void startStopTimer(bool b);
    void sendDelayedModificationChanged();

private:

    int delay_;
    QTimer *timer;
    
};

#endif // TEXTDOCUMENT_H
