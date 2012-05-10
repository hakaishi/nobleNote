#ifndef NOBLENOTE_H
#define NOBLENOTE_H

#include "ui_mainwindow.h"
#include <QSystemTrayIcon>
#include <QSplitter>
#include <QFileSystemModel>
#include <QListView>

/**
 * @brief note taking application main window
 *
 *  nb short for notebook
 *  n short for note
 *  f short for folder
 */

class Preferences;

class NobleNote : public QMainWindow, public Ui::NobleNote {
     Q_OBJECT // important for creating own singals and slots
 
     public:
      NobleNote();
      ~NobleNote();
     private:
      QString         text, str, origPath;
      QSplitter       *splitter;
      QFileSystemModel *folderModel, *noteModel;
      QListView       *folderList, *noteList;
      QAction         *minimize_restore_action, *quit_action;
      QIcon           icon;
      QSystemTrayIcon *TIcon;
      QMenu           *iMenu;
      Preferences     *pref;

     private slots:
      void changeRootIndex();
      void setCurrentFolder(const QModelIndex &ind);
      void iconActivated(QSystemTrayIcon::ActivationReason reason);
      void tray_actions();
      void openNote(const QModelIndex &ind = QModelIndex());
      void showContextMenuF(const QPoint &pos);
      void showContextMenuN(const QPoint &pos);
      void newFolder();
      void newNote();
      void renameFolder();
      void renameNote();
      void removeFolder();
      void removeNote();
      void setFirstFolderCurrent(QString path); // select first folder

     protected:
      void keyPressEvent(QKeyEvent* kEvent);
      virtual void closeEvent(QCloseEvent* window_close);
      virtual void showEvent(QShowEvent* window_show);
      virtual void hideEvent(QHideEvent* window_hide);
};

#endif
