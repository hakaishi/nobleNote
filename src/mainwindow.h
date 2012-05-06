#ifndef NOBLENOTE_H
#define NOBLENOTE_H

#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QtGlobal>
#include <QSettings>
#include <QSplitter>
#include <QFileSystemModel>
#include <QListView>

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
      void showContextMenu(const QPoint &pos);
      void newF();
      void newN();
      void renameF();
      void renameN();
      void removeFolder();
      void removeNote();

     signals:

     protected:
      //void mouseDoubleClickEvent(QMouseEvent *event){ Q_UNUSED(event); }
};

#endif
