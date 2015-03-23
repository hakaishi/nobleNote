/* nobleNote, a note taking application
 * Copyright (C) 2015 Christian Metscher <hakaishi@web.de>,
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

#ifndef FILESYSTEMMODEL_H
#define FILESYSTEMMODEL_H

#include <QFileSystemModel>
#include <QSettings>
#include <QMimeData>
#include <QUrl>
#include <QMessageBox>

/**
 * @brief  overwritten base class that circumvents a bug in QFileSystemodel which causes
 * the flags method to never return a Qt::ItemIsEditable
 *  because it checks for the permission QFile::WriteUser which seems to be always false
 *  if Qt::ItemIsEditable is never set, setData or edit methods will always fail
 */

class FileSystemModel : public QFileSystemModel
{
    Q_OBJECT
public:
    explicit FileSystemModel(QObject *parent = 0) : QFileSystemModel(parent) {}
    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        return QFileSystemModel::flags(index) | Qt::ItemIsEditable;
    }

    // disable drops between items and elsewhere in the viewport
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
    {
        // is drop into "emtpy space"
        bool isRootFolder = this->index(QSettings().value("root_path").toString()) == parent;

        // row == -1 && column == -1 dropped directly on item or on "empty space"
        if((row == -1 && column == -1)  && !isRootFolder)
        {
            QStringList files;
            foreach(QUrl url, data->urls())
                   if(!url.toLocalFile().isEmpty())
                     files << url.toLocalFile();

            if(files.isEmpty()) //no local files
              return false;

            if(this->index(QFileInfo(files.first()).absolutePath()) == parent)
              return false; //dropped in the same folder they are in

            QDir dirs(QSettings().value("root_path").toString());
            QList<QFileInfo> dirList = dirs.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name);

            //Search for the right dir
            QString parentPath;
            foreach(QFileInfo file, dirList)
                   if(this->index(QFileInfo(file).absoluteFilePath()) == parent)
                     parentPath = QFileInfo(file).absoluteFilePath();

            if(dirList.contains(QFileInfo(files.first()).absolutePath())) //check if files come from outside the note folders
               action = Qt::MoveAction;

            //remove all file titles that don't exist in the target folder, because they will be sucessfully dropped
            foreach(QString file, files)
            {
               #ifdef Q_OS_WIN32
                QString path = parentPath + "\\" + QFileInfo(file).fileName();
               #else
                QString path = parentPath + "/" + QFileInfo(file).fileName();
               #endif
                if(!QFileInfo(path).exists())
                   files.removeOne(file);
            }

            bool dropped = QFileSystemModel::dropMimeData(data,action,row,column,parent);

            if(!dropped)
            {
                QString existingFiles;
                foreach(QString file, files)
                       existingFiles += file + "\n";
                if(!existingFiles.isEmpty())
                {
                  QString title = tr("Files could not be dropped");
                  QString text = tr("The files could not be dropped because files of the same names are already existing in this notebook:\n\n%1").arg(QDir::toNativeSeparators(existingFiles));
                  if(files.size() == 1)
                  {
                    title = tr("File could not be dropped");
                    text = tr("The file could not be dropped because a file with the same name already exists in this notebook:\n\n%1").arg(QDir::toNativeSeparators(existingFiles));
                  }
                  QMessageBox::warning(0, title, text);
                }
                return false;
            }
            else
               return true; //files sucessfully dropped
        }
        return false; // dropped between items
    }
};

#endif // FILESYSTEMMODEL_H
