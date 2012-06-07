#ifndef FINDFILEMODEL_H
#define FINDFILEMODEL_H

#include <QStandardItemModel>
#include <QFile>
#include <QFileInfo>
#include <QtConcurrentFilter>
#include <QFuture>
#include <QFutureWatcher>
#include <QMimeData>

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
    static QStringList find0(const QString &searchName, const QString &searchText,const QString& path); // searchName and searchText can be null QStrings
    void findInFiles(const QString &path, const QString& fileName, const QString &content);

    QStringList mimeTypes() const;
    QMimeData * mimeData(const QModelIndexList &indexes) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role); // returns if rename succeeded, overwritten to enable QAbstractItemView::edit

private:
    struct FileContains // functor that checks if a text file (read as html) contains a given text
    {
        QString fileName;
        QString content;
        bool operator()(const QString& htmlFilePath);
    private:
        bool fileContentContains(const QString& htmlFilePath);
    };

    QFuture<QString> future;
    FileContains fileContainsFunctor;

    QFutureWatcher<QString> futureWatcher;

private slots:
    void findInFilesFinished(); // populate model with find results
};

#endif // FINDFILEMODEL_H
