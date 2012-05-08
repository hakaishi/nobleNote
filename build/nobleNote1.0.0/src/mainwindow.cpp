#include "mainwindow.h"
#include "note.h"
#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QModelIndex>
#include <QInputDialog>
#include <QMouseEvent>
//#include <QSettings>
#include "filesystemmodel.h"


NobleNote::NobleNote(){

     setupUi(this);

//TODO: create icon, system tray icon.
//TODO: enable drag and drop.
//TODO: open entry for notes.

     origPath = QDir::homePath() + "/.nobleNote";
     QDir nbDir(origPath);
     if(!nbDir.exists())
       nbDir.mkdir(origPath);

     splitter = new QSplitter(centralwidget);
     gridLayout->addWidget(splitter, 0, 0);

     fModel = new FileSystemModel(this);
     fModel->setRootPath(origPath);
     fModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);


     nModel = new FileSystemModel(this);
     nModel->setRootPath(origPath); //just as an example
     nModel->setFilter(QDir::Files);

     nbList = new QListView(splitter); //note book list
     nbList->setModel(fModel);
     nbList->setRootIndex(fModel->index(origPath));
     nbList->setContextMenuPolicy(Qt::CustomContextMenu);
     //nbList->setEditTriggers(QListView::NoEditTriggers);
     nList = new QListView(splitter);  //note list
     nList->setEditTriggers(QListView::NoEditTriggers);
     nList->setModel(nModel);
     nList->setRootIndex(nModel->index(origPath));
     nList->setContextMenuPolicy(Qt::CustomContextMenu);

//TODO: make it possible to import notes from some other folder or even another program

//TODO?: Make it possible to change the search path for note books and notes?

  /* QString file(origPath + "/nobleNote.conf");
     QSettings settings(file, QSettings::IniFormat);
     if(!settings.isWritable()){
       QTextStream myOutput;
       myOutput << "W: nobelNote.conf is not writable!" << endl;
       qApp->quit();
     }*/

//TODO: rather selectionChanged as clicked for setNewFolder.

     connect(actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
     connect(nbList, SIGNAL(clicked(const QModelIndex &)), this, SLOT(setNewFolder(const QModelIndex &)));
     connect(nList, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(nbAction(const QModelIndex &)));
     connect(nbList, SIGNAL(customContextMenuRequested(const QPoint &)),
    this, SLOT(showContextMenuF(const QPoint &)));
     connect(nList, SIGNAL(customContextMenuRequested(const QPoint &)),
    this, SLOT(showContextMenuN(const QPoint &)));
}

NobleNote::~NobleNote(){}

void NobleNote::setNewFolder(const QModelIndex &ind){
     nList->setRootIndex(nModel->setRootPath(nModel->fileInfo(ind).filePath()));
}

void NobleNote::nbAction(const QModelIndex &ind){
     QString newPath = nModel->fileInfo(ind).filePath();
     QFile file(newPath);
     if(!file.open(QIODevice::ReadOnly))
       return;
     QTextStream stream(&file);
     text = stream.readAll();
     file.close();

     Note *notes = new Note(this);
     notes->text = text;
     notes->notesPath = newPath;
     notes->show();
}

void NobleNote::newF(){
     QDir dir(origPath + "/" + tr("new folder"));
     dir.mkdir(origPath + "/" + tr("new folder"));
}

void NobleNote::newN(){
     QFile file(nModel->rootPath() + "/" + tr("new note"));
     if(!file.open(QIODevice::WriteOnly))
       return;
     file.close();
}

void NobleNote::renameF(){
    nbList->edit(nbList->currentIndex());
}

void NobleNote::renameN(){
    nList->edit(nList->currentIndex());
}

void NobleNote::removeFolder(){
     fModel->rmdir(nbList->currentIndex());
//TODO: check why:
//QInotifyFileSystemWatcherEngine::addPaths: inotify_add_watch failed: Datei oder Verzeichnis nicht gefunden
//QFileSystemWatcher: failed to add paths: /home/hakaishi/.nobleNote/new folder
}

void NobleNote::removeNote(){
     nModel->remove(nList->currentIndex());
}

void NobleNote::showContextMenuF(const QPoint &pos){
     QPoint globalPos = this->mapToGlobal(pos);

     QMenu* menu = new QMenu(this);
     QAction* addNewF = new QAction(tr("New &folder"), this);
     QAction* renameF = new QAction(tr("R&ename folder"), this);
     QAction* removeFolder = new QAction(tr("&Remove folder"), this);

     connect(addNewF, SIGNAL(triggered()), this, SLOT(newF()));
     connect(renameF, SIGNAL(triggered()), this, SLOT(renameF()));
     connect(removeFolder, SIGNAL(triggered()), this, SLOT(removeFolder()));

     menu->addAction(addNewF);
     menu->addAction(renameF);
     menu->addAction(removeFolder);
    
     menu->exec(globalPos);
}

void NobleNote::showContextMenuN(const QPoint &pos){
     QPoint globalPos = this->mapToGlobal(pos);

     QMenu* menu = new QMenu(this);
     QAction* open = new QAction(tr("&Open note"), this);
     QAction* addNewN = new QAction(tr("New &note"), this);
     QAction* renameN = new QAction(tr("Ren&ame note"), this);
     QAction* removeNote = new QAction(tr("Re&move note"), this);

     connect(open, SIGNAL(triggered()), this, SLOT(openN()));
     connect(addNewN, SIGNAL(triggered()), this, SLOT(newN()));
     connect(renameN, SIGNAL(triggered()), this, SLOT(renameN()));
     connect(removeNote, SIGNAL(triggered()), this, SLOT(removeNote()));

     menu->addAction(open);
     menu->addSeparator();
     menu->addAction(addNewN);
     menu->addAction(renameN);
     menu->addAction(removeNote);
    
     menu->exec(globalPos);
}
