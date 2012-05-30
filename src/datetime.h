#ifndef DATETIME_H
#define DATETIME_H

#include <QString>
#include <QDateTime>

class DateTime
{
public:
    static QString getTimeZoneOffset(QDateTime dt1);
    static QString toISO8601(QDateTime dt);
};

#endif // DATETIME_H
