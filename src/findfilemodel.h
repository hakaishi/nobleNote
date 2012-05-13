#ifndef FINDFILEMODEL_H
#define FINDFILEMODEL_H

#include <QStandardItemModel>
#include <QFile>
#include <QFileInfo>

/**
 * @brief a model that shows a list of files
 *
 */

class FindFileModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit FindFileModel(QObject *parent = 0);

    QString fileName( const QModelIndex & index ) const;
    QString filePath ( const QModelIndex & index ) const;
    bool remove ( const QModelIndex & index ) const;
    QFileInfo	fileInfo ( const QModelIndex & index ) const;
    void appendFile(QString file); // append file with full path
    // icons with QFileIconProvider ()
    
};

#endif // FINDFILEMODEL_H
