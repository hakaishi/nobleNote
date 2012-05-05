#include "mainwindow.h"
#include "note.h"
#include <QDebug>
#include <QTextStream>
#include <QFile>

NobleNote::NobleNote(){

     setupUi(this);

     origPath = QDir::homePath() + "/.nobleNote";
     QDir nbDir(origPath);
     if(!nbDir.exists())
       nbDir.mkdir(origPath);

     splitter = new QSplitter(centralwidget);
     gridLayout->addWidget(splitter, 0, 0);

     dModel = new QFileSystemModel(this);
     dModel->setRootPath(origPath);
     dModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

     fModel = new QFileSystemModel(this);
     fModel->setRootPath(origPath); //just as an example

     nbList = new QListView(splitter); //note book list
     nbList->setModel(dModel);
     nbList->setRootIndex(dModel->index(origPath));
     nList = new QListView(splitter);  //note list
     nList->setModel(fModel);
     nList->setRootIndex(fModel->index(origPath));

  /* QString file(origPath + "/nobleNote.conf");
     QSettings settings(file, QSettings::IniFormat);
     if(!settings.isWritable()){
       QTextStream myOutput;
       myOutput << "W: nobelNote.conf is not writable!" << endl;
       qApp->quit();
     }*/

     connect(actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
     connect(nbList, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(setNewFolder(const QModelIndex &)));
     connect(nList, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(nbAction(const QModelIndex &)));
     connect(nbList, SIGNAL(customContextMenuRequested(const QPoint &)),
    this, SLOT(showContextMenu(const QPoint &)));
     connect(nList, SIGNAL(customContextMenuRequested(const QPoint &)),
    this, SLOT(showContextMenu(const QPoint &)));
}

NobleNote::~NobleNote(){}

void NobleNote::setNewFolder(const QModelIndex &ind){
     nList->setRootIndex(fModel->index(fModel->fileInfo(ind).filePath()));
}

void NobleNote::nbAction(const QModelIndex &ind){
     QString newPath = fModel->fileInfo(ind).filePath();
     if(fModel->fileInfo(ind).isDir())
       nList->setRootIndex(fModel->index(newPath));
     if(fModel->fileInfo(ind).isFile()){
       QFile file(newPath);
       if(!file.open(QIODevice::ReadOnly))
         return;
       QTextStream stream(&file);
       text = stream.readAll();  //TODO: there is uncalled text/html (after save)
       file.close();

       Note *notes = new Note(this);
       notes->text = text;
       notes->notesPath = newPath;
       notes->show();
     }
}

void NobleNote::newF(){

}

void NobleNote::newN(){

}

void NobleNote::renameItem(){

}

void NobleNote::removeItem(){

}

void NobleNote::showContextMenu(const QPoint &pos){ //TODO: won't work?
     QPoint globalPos = this->mapToGlobal(pos);

     QMenu* menu = new QMenu(this);
     QAction* addNewF = new QAction(tr("New Folder"), this);
     QAction* addNewN = new QAction(tr("New &Note"), this);
     QAction* rename = new QAction(tr("R&ename Note"), this);
     QAction* removeI = new QAction(tr("&Remove Note"), this);

     connect(addNewF, SIGNAL(triggered()), this, SLOT(newF()));
     connect(addNewN, SIGNAL(triggered()), this, SLOT(newN()));
     connect(rename, SIGNAL(triggered()), this, SLOT(renameItem()));
     connect(removeI, SIGNAL(triggered()), this, SLOT(removeItem()));

     menu->addAction(addNewF);
     menu->addAction(addNewN);
     menu->addAction(rename);
     menu->addAction(removeI);
    
     menu->exec(globalPos);
}
