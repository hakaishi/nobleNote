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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "ui_preferences.h"
#include <QSettings>
#include <QProgressDialog>
#include <QFutureWatcher>

class Preferences : public QDialog, public Ui::Preferences {
     Q_OBJECT
 
     public:
      Preferences(QWidget *parent = 0);
      ~Preferences();
      QString rootPath;

     private:
      QSettings *settings;
      QString    originalRootPath;
      QProgressDialog *dialog;
      QFutureWatcher<QString> *futureWatcher;
      struct getFiles
      {
        QStringList notes;
        QStringList notesUuids;
        bool operator()(const QString& backupAndUuid);
      private:
        bool removeBackup(const QString& backupAndUuid);
        bool removeSettingsUuid(const QString& backupAndUuid);
      };
      getFiles getFilesFunctor;

     private slots:
      void saveSettings();
      void openDir();
      void deleteOldBackupsAndFileEntries();

     signals:
      void sendPathChanged();

     protected:
      virtual void showEvent(QShowEvent* show_pref);
};

#endif //PREFERENCES_H
