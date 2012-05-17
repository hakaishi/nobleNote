#ifndef NOBLENOTE_H
#define NOBLENOTE_H

#include "ui_mainwindow.h"
#include <QSystemTrayIcon>
#include <QSplitter>
#include <QFileSystemModel>
#include <QListView>
#include <QAbstractItemModel>
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>


/**
 * @brief note taking application main window
 *
 *  nb short for notebook
 *  n short for note
 *  f short for folder
 */

//#define NO_SYSTEM_TRAY_ICON

class Preferences;
class FindFileModel;
class FindFileSystemModel;
class FileSystemModel;
class LineEdit;
class Note;

class NobleNote : public QMainWindow, public Ui::NobleNote {
     Q_OBJECT // important for creating own singals and slots
 
     public:
      NobleNote();
      ~NobleNote();

     private:
      QString         text;
      QSplitter       *splitter;
      LineEdit        *searchName, *searchText;
      FileSystemModel *folderModel;
      FileSystemModel *noteFSModel;
      FindFileSystemModel * noteModel;
      QListView       *folderList;
      QListView         *noteList;
      QAction         *quit_action;
      QIcon           icon;
      Preferences     *pref;
      QAction         *minimize_restore_action;
      QHBoxLayout     *hBoxLayout;
      QToolButton     *showHideAdvancedSearchButton;
      QLabel          *findLabel;
      QFrame          *hLine;
      bool            searchBoolean;
      FindFileModel   *findNoteModel;
      QList<Note*>     openNotes;

#ifndef NO_SYSTEM_TRAY_ICON
      QMenu           *iMenu;
      QSystemTrayIcon *TIcon;
#endif

     private slots:
      //void changeRootIndex();
      void setCurrentFolder(const QModelIndex &ind);

#ifndef NO_SYSTEM_TRAY_ICON
      void iconActivated(QSystemTrayIcon::ActivationReason reason);
      void tray_actions();
#endif

      void find();
      void showHideAdvancedSearch();
      void openNote(const QModelIndex &ind = QModelIndex());
      void removeNoteFromOpenList(QObject * object);
      void showContextMenuF(const QPoint &pos);
      void showContextMenuN(const QPoint &pos);
      void newFolder();
      void newNote();
      void renameFolder();
      void renameNote();
      void removeFolder();
      void removeNote();
      void selectFirstFolder(QString path); // select first folder

     protected:
      virtual void closeEvent(QCloseEvent* window_close);
      virtual void showEvent(QShowEvent* window_show);
      virtual void hideEvent(QHideEvent* window_hide);
};

#endif
