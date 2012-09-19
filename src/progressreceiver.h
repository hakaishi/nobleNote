#ifndef PROGRESSRECEIVER_H
#define PROGRESSRECEIVER_H

#include <QObject>
#include <QEvent>
#include <QTime>

/**
 * @brief The ProgressReceiver class can be used to set the values of a progress bar
 * if a lenghty operation runs in a separate thread. You must call postProgressEvent() in the separate thread.
 * the separate thread does not need an event queue.
 */


class ProgressReceiver : public QObject
{
    Q_OBJECT
public:
    explicit ProgressReceiver(QObject *parent = 0);

    // the minimum intervall beweteen two events that are received by progress receiver
    void setInterval(int interval){ interval_ = interval; }
    int interval() const { return interval_; }

    // the current value of the internal progress counter
    void setValue(int value){ value_ = value; }
    int value() const { return value_; }

    // post a progress event to itself that will be posted on a event queue at least every interval ms
    // and will be processed by this object's event method which will increase the value() and send valueChanged
    // this method is not thread safe, but it is usually called in 1 other thread
    void postProgressEvent();


    //override
    virtual bool event ( QEvent * e );
    
signals:

    void valueChanged(int value);
    
public slots:


private:
    struct ProgressEvent : public QEvent
    {
        ProgressEvent( Type type) : QEvent(type), value(0)
        {
        }

        int value;
    };

    int interval_;
    int value_;
    QTime time_;
    
};

#endif // PROGRESSRECEIVER_H
