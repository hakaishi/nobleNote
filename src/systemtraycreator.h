#ifndef SYSTEMTRAYCREATOR_H
#define SYSTEMTRAYCREATOR_H

#include <QMenu>
#include <QObject>

class SystemTrayCreator : public QObject
{
    Q_OBJECT
public:
    explicit SystemTrayCreator(QObject *parent = nullptr);

     void populateMenu(QMenu * menu);

signals:

     void noteClicked(QString noteFilePath);

};

#endif // SYSTEMTRAYCREATOR_H
