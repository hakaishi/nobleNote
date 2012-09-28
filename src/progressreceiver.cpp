#include "progressreceiver.h"
#include <qcoreevent.h>
#include <QCoreApplication>
#include <QTime>

ProgressReceiver::ProgressReceiver(QObject *parent) :
    QObject(parent)
{
    interval_ = 20;
    value_ = 0;
    time_  = QTime::currentTime();
}

// if the event user type collides with an existing user type, the static_cast in the
// event method fails
const int userTypeOffset = 314;

void ProgressReceiver::postProgressEvent()
{
        value_ = ++value_; // the number of the currently processed item

    ProgressEvent * me = new ProgressEvent(static_cast<QEvent::Type>(QEvent::User + userTypeOffset));
    me->value = value_;

    // only report periodically
    if(QTime::currentTime() > time_)
    {
        time_ = time_.addMSecs(interval_);

        // ProgressReceiver now receives a event with the current progress
        QCoreApplication::postEvent(this,me);
    }
}

bool ProgressReceiver::event(QEvent *e)
{
     ProgressEvent * me = 0; // new nullpointer constant in c++11

    // is MyEvent type?
    if(e->type() == QEvent::User +userTypeOffset)
    {
         me = static_cast<ProgressEvent*>(e); // downcast

        // report progress
        if(me)
        {
            emit valueChanged(me->value);
            return true;
        }
    }

    // important!
    return QObject::event(e);
}
