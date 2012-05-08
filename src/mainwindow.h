#ifndef NOBLENOTE_H
#define NOBLENOTE_H

#include "ui_mainwindow.h"
#include <QFileDialog>
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

class NobleNote : public QMainWindow, public Ui::NobleNote {
     Q_OBJECT // important for creating own singals and slots
 
     public:
      NobleNote();
      ~NobleNote();

     private:
      QString text, str, origPath;
      QSplitter *splitter;
      QFileSystemModel *fModel, *nModel;
      QListView *nbList, *nList;

     private slots:
      void setNewFolder(const QModelIndex &ind);
      void nbAction(const QModelIndex &ind);
      void showContextMenuF(const QPoint &pos);
      void showContextMenuN(const QPoint &pos);
      void newF();
      void newN();
      void renameF();
      void renameN();
      void removeFolder();
      void removeNote();
};

#endif
