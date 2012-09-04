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
#include <QSyntaxHighlighter>
#include <QSystemTrayIcon>
#include <QSplitter>
#include <QFileSystemModel>
#include <QListView>
#include <QAbstractItemModel>
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QPointer>
#include <QProgressDialog>
#include <QFutureWatcher>


/**
 * @brief note taking application main window
 *
 *  nb short for notebook
 *  n short for note
 *  f short for folder
 */

//#define NO_SYSTEM_TRAY_ICON

class Welcome;
class MainWindowToolbar;
class Preferences;
class FindFileModel;
class FindFileSystemModel;
class FileSystemModel;
class LineEdit;
class Note;
class Highlighter;

class MainWindow : public QMainWindow, public Ui::NobleNote {
     Q_OBJECT // important for creating own singals and slots
 
     public:
      MainWindow();
      ~MainWindow();


public slots:
    void quit();
private:
      Welcome         *welcome;
      MainWindowToolbar         *toolbar;
      QSplitter       *splitter;
      LineEdit        *searchName, *searchText;
      FindFileSystemModel *folderModel;
      FileSystemModel *noteFSModel;
      FindFileSystemModel * noteModel;
      QListView       *folderView;
      QListView         *noteView;
      QAction         *quit_action;
      QIcon           icon;
      Preferences     *pref;
      QAction         *minimizeRestoreAction;
      QHBoxLayout     *hBoxLayout;
      FindFileModel   *findNoteModel;
      QList<QPointer<Note> >     openNotes; // every access to openNotes must check for null pointers
      Highlighter *highlighter;
      QFutureWatcher<void> *future;
      QProgressDialog *dialog;

#ifndef NO_SYSTEM_TRAY_ICON
      QMenu           *iMenu;
      QSystemTrayIcon *TIcon;
#endif

      Note * noteWindow(const QString & filePath); // return the open note window for the note at filePath

     private slots:
      void checkAndSetFolders();
      void changeRootIndex();

      void onFolderSelectionChanged( const QItemSelection & selected, const QItemSelection & deselected );

#ifndef NO_SYSTEM_TRAY_ICON
      void iconActivated(QSystemTrayIcon::ActivationReason reason);
      void tray_actions();
#endif

      void find();
      void openNote(const QModelIndex &ind = QModelIndex());
      void showContextMenuFolder(const QPoint &pos);
      void showContextMenuNote(const QPoint &pos);
      void newFolder();
      void newNote();
      void renameFolder();
      void renameNote();
      void removeFolder();
      void removeNote();
      void importXmlNotes();
      void selectFirstFolder(QString path); // select first folder
      void folderRenameFinished( QWidget * editor, QAbstractItemDelegate::EndEditHint hint = QAbstractItemDelegate::NoHint ); // reloads current folder
      void noteRenameFinished(const QString &path, const QString &oldName, const QString &newName); // updates window title
      void about();

     protected:
      void keyPressEvent(QKeyEvent *k);
      virtual void closeEvent(QCloseEvent* window_close);
      virtual void showEvent(QShowEvent* window_show);
      virtual void hideEvent(QHideEvent* window_hide);
};

#endif
