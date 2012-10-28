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

#ifndef NOBLENOTE_H
#define NOBLENOTE_H

#include "ui_mainwindow.h"
#include "htmlnotewriter.h"
#include "progressreceiver.h"
#include "noteimporter.h"
#include <QSyntaxHighlighter>
#include <QSystemTrayIcon>
#include <QSplitter>
#include <QListView>
#include <QFileSystemModel>
#include <QAbstractItemModel>
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QPointer>
#include <QProgressDialog>
#include <QFutureWatcher>
#include <QFileDialog>

/**
 * @brief note taking application main window
 *
 *  nb short for notebook
 *  n short for note
 *  f short for folder
 */

//#define NO_SYSTEM_TRAY_ICON

class Welcome;
class Preferences;
class FindFileModel;
class FindFileSystemModel;
class FileSystemModel;
class LineEdit;
class Note;
class Highlighter;
class ProgressReceiver;
class Backup;
class FlickCharm;

class MainWindow : public QMainWindow, public Ui::NobleNote {
     Q_OBJECT // important for creating own singals and slots
 
     public:
      MainWindow();

public slots:
    void quit();
private:
      Welcome         *welcome;
      QSplitter       *splitter;
      LineEdit        *searchName, *searchText;
      FindFileSystemModel *folderModel;
      FileSystemModel *noteFSModel;
      FindFileSystemModel *noteModel;
      QListView        *folderView, *noteView;
      QAction         *quit_action, *minimizeRestoreAction;
      QPointer<Preferences> pref;
      QHBoxLayout     *hBoxLayout;
      FindFileModel   *findNoteModel;
      QList<QPointer<QWidget> > openNotes; // every access to openNotes must check for null pointers
      QPointer<Backup> backup;

      FlickCharm      *flickCharm; // kinetic scrolling for both list views and all notes
      QStringList     shortcutNoteList;

      NoteImporter * noteImporter;

#ifndef NO_SYSTEM_TRAY_ICON
      QMenu           *iMenu;
      QSystemTrayIcon *TIcon;
#endif

      Note * noteWindow(const QString & filePath); // return the open note window for the note at filePath

     private slots:
      void writeBackupDirPath();
      void changeRootIndex();
      void makeStandardPaths();
      void enableNoteMenu(const QItemSelection &selected, const QItemSelection &deselected);
      void showPreferences();
      void showBackupWindow();
      void folderActivated(const QModelIndex &selected);
      void folderActivated(const QItemSelection &selected, const QItemSelection &deselected); //Wrapper
      void noteActivated(const QModelIndex &selected);
      void noteActivated(const QItemSelection &selected, const QItemSelection &deselected); //Wrapper

#ifndef NO_SYSTEM_TRAY_ICON
      void iconActivated(QSystemTrayIcon::ActivationReason reason);
      void tray_actions();
#endif

      void find();
      void openNote(const QModelIndex &ind);
      void openAllNotes();
      void openNoteSource();
      void showContextMenuFolder(const QPoint &pos);
      void showContextMenuNote(const QPoint &pos);
      void newFolder();
      void newNote();
      void renameFolder();
      void renameNote();
      void removeFolder();
      void removeNote();
      void setKineticScrollingEnabled(bool b); // enable flickCharm
      void folderRenameFinished( QWidget * editor, QAbstractItemDelegate::EndEditHint hint = QAbstractItemDelegate::NoHint ); // reloads current folder
      void noteRenameFinished(const QString &path, const QString &oldName, const QString &newName); // updates window title
      void getCutFiles();
      void pasteFiles();
      void about();

     protected:
      void keyPressEvent(QKeyEvent *k);
      virtual void closeEvent(QCloseEvent* window_close);
      virtual void showEvent(QShowEvent* window_show);
      virtual void hideEvent(QHideEvent* window_hide);
};

#endif
