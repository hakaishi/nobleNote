#ifndef FINDFILEMODEL_H
#define FINDFILEMODEL_H

#include <QStandardItemModel>

class FindFileModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit FindFileModel(QObject *parent = 0);
    // icons with QFileIconProvider ()
    
};

#endif // FINDFILEMODEL_H
