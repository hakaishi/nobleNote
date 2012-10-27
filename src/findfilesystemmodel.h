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

#ifndef FINDFILESYSTEMMODEL_H
#define FINDFILESYSTEMMODEL_H

#include <QSortFilterProxyModel>
#include <QFileInfo>

/**
 * @brief  proxy model for both QFileSystemModel and for FindFileModel
 *          that allows access to some methods that exist in both classes
 *          call setSourceModel to an instance of one of these models,
 *          the sourceModel() will be automatically cast to one of these two models
 *
 *          Important: always use mapToSource and mapFromSource for QModelIndexes
 */

class FindFileSystemModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit FindFileSystemModel(QObject *parent = 0);
    QString fileName(const QModelIndex & index) const;
    QStringList fileNames(const QList<QModelIndex> & index) const;
    QString filePath(const QModelIndex & index) const;
    bool	rmdir ( const QModelIndex & index ) const;
    QModelIndex	mkdir ( const QModelIndex & parent, const QString & name );
    bool remove(const QModelIndex & index) const;
    bool removeList(const QList<QModelIndex> & index) const;
    QFileInfo fileInfo(const QModelIndex & index) const;
    void appendFile(QString filePath); // append file with full path
    QModelIndex setRootPath(const QString & newPath);
    QString rootPath() const;
    void clear();
    void findInFiles(const QString& fileName, const QString &content, const QString &path);
    QModelIndex index ( const QString & path, int column = 0 ) const; // wrapper method for the corresponding QFileSystemModel method
    void copyNotesToBackupDir(const QModelIndexList &indexes) const; // reads the uuid from each note and copys the note to the backup dir with the uuid as its name

    static void copyNotesToBackupDir(const QFileInfoList &fileInfos);
    static bool removeList(const QFileInfoList &fileInfos);
};

#endif // FINDFILESYSTEMMODEL_H
