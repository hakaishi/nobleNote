/* nobleNote, a note taking application
 * Copyright (C) 2019 Christian Metscher <hakaishi@web.de>,
                      Fabian Deuchler <Taiko000@gmail.com>

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.

 * nobleNote is licensed under the MIT, see `http://copyfree.org/licenses/mit/license.txt'.
 */

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
