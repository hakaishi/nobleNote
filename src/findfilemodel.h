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

    QString fileName(const QModelIndex & index) const;
    QString filePath(const QModelIndex & index) const;
    bool remove(const QModelIndex & index);
    QFileInfo fileInfo(const QModelIndex & index) const;
    void appendFile(QString filePath); // append file with full path
    QStringList find(const QString &searchName, const QString &searchText,const QString& path); // searchName and searchText can be null QStrings

    bool setData(const QModelIndex &index, const QVariant &value, int role); // returns if rename succeeded, overwritten to enable QAbstractItemView::edit

private:
    QStringList findFiles(const QStringList &files, const QString &text, const QString &path);
};

#endif // FINDFILEMODEL_H
