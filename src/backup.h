/* nobleNote, a note taking application
 * Copyright (C) 2020 Christian Metscher <hakaishi@web.de>,
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

#ifndef BACKUP_H
#define BACKUP_H

#include "htmlnotereader.h"
#include "abstractnotereader.h"
#include "progressreceiver.h"
#include <QFileInfo>
#include <QTextDocument>
#include <QProgressDialog>
#include <QFutureWatcher>

class ProgressReceiver;
class Trash;

///
/// \brief The Backup class asynchronously reads note backup files from the file system
/// and shows a Trash window where trashed notes can be inspected and restored
///
class Backup : public QObject {
     Q_OBJECT
 
     public:
      Backup(QWidget *parent); // creating an instances launches the async file loading and widget creation immediately
      QWidget *parent_;

     private:
      QTextDocument *document;
      QPushButton   *deleteOldButton;
      QStringList   noteFiles;
      QFutureWatcher<QString> *future1;
      QFutureWatcher<QPair<QString, QStringList>> *future2;
      ProgressReceiver *progressReceiver1, *progressReceiver2;
      QProgressDialog  *progressDialog1, *progressDialog2;
      QList<QFileInfo> backupFiles; //don't ever use a local stack variable
      QHash<QString,QStringList> backupDataHash;
      Trash         *trash;

      // functor that reads uuids
      struct GetUuid
      {
           ProgressReceiver *p;

           typedef QString result_type;

           QString operator()(QString file)
           {
                p->postProgressEvent();
                return HtmlNoteReader::uuid(file).toString();
           }
      };


      // functor used for multithreaded note file reading method
      struct SetupBackup
      {
          ProgressReceiver *p;

          // Return key/value pair instead of mutating shared QHash
          typedef QPair<QString, QStringList> result_type;

          result_type operator()(const QFileInfo &file)
          {
              QStringList data;
              HtmlNoteReader reader(file.absoluteFilePath());
              reader.read();

              QString title = reader.title();
              if (title.isEmpty())
                  title = QObject::tr("deleted note");

              data << title << file.absoluteFilePath() << reader.html();

              if (p) p->postProgressEvent();

              return qMakePair(file.absoluteFilePath(), data);
          }
      };


      GetUuid getUuid;
      SetupBackup setupBackup;

     private slots:
      void getNotes();
      void setupBackups();
      void showTrash();
};

#endif //BACKUP_H
