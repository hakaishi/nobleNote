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

void ProgressReceiver::postProgressEvent()
{
        value_ = ++value_; // the number of the currently processed item

    ProgressEvent * me = new ProgressEvent(static_cast<QEvent::Type>(QEvent::User + 1));
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
    if(e->type() == QEvent::User +1)
    {
         me = dynamic_cast<ProgressEvent*>(e); // downcast

        // report progress
        if(me)
        {
           // qDebug() << (QString("Progress at: ") + QString::number(me->value));

            emit valueChanged(me->value);
            return true;
        }
    }

    // important!
    return QObject::event(e);
}
