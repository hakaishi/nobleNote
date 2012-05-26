#ifndef DATETIME_H
#define DATETIME_H

namespace DateTime
{
    QString getTimeZoneOffset(QDateTime dt1)
    {
        QDateTime dt2 = dt1.toUTC();
        dt1.setTimeSpec(Qt::UTC);

        int offset = dt2.secsTo(dt1) / 3600;

        if (offset > 0)
            return QString().sprintf("+%02d:00",offset);

        return QString().sprintf("%02d:00",offset);
    }

    QString toISO8601(QDateTime dt)
    {
        return dt.toString(Qt::ISODate)+ "." + dt.toString("zzz") + "0000"  + getTimeZoneOffset(dt);
    }
}

#endif // DATETIME_H
