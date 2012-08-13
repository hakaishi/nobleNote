/* nobleNote, a note taking application
 * Copyright (C) 2012 Christian Metscher <hakaishi@web.de>,
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
