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

     fModel = new QFileSystemModel(this);
     fModel->setRootPath(origPath);
     fModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

     nModel = new QFileSystemModel(this);
     nModel->setRootPath(origPath); //just as an example
     nModel->setFilter(QDir::Files);

     nbList = new QListView(splitter); //note book list
     nbList->setModel(fModel);
     nbList->setRootIndex(fModel->index(origPath));
     nbList->setContextMenuPolicy(Qt::CustomContextMenu);
     nList = new QListView(splitter);  //note list
     nList->setModel(nModel);
     nList->setRootIndex(nModel->index(origPath));
     nList->setContextMenuPolicy(Qt::CustomContextMenu);

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

void NobleNote::renameF(){ //TODO: somehow add flag to make it editable
     nbList->currentIndex().flags() | Qt::ItemIsEditable;
     nbList->edit(nbList->currentIndex());
}

void NobleNote::renameN(){ //TODO: somehow add flag to make it editable
     nList->currentIndex().flags() | Qt::ItemIsEditable;
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

void NobleNote::showContextMenu(const QPoint &pos){
     QPoint globalPos = this->mapToGlobal(pos);

     QMenu* menu = new QMenu(this);
     QAction* addNewF = new QAction(tr("New &folder"), this);
     QAction* addNewN = new QAction(tr("New &note"), this);
     QAction* renameF = new QAction(tr("R&ename folder"), this);
     QAction* renameN = new QAction(tr("Ren&ame note"), this);
     QAction* removeFolder = new QAction(tr("&Remove folder"), this);
     QAction* removeNote = new QAction(tr("Re&move note"), this);

     connect(addNewF, SIGNAL(triggered()), this, SLOT(newF()));
     connect(addNewN, SIGNAL(triggered()), this, SLOT(newN()));
     connect(renameF, SIGNAL(triggered()), this, SLOT(renameF()));
     connect(renameN, SIGNAL(triggered()), this, SLOT(renameN()));
     connect(removeFolder, SIGNAL(triggered()), this, SLOT(removeFolder()));
     connect(removeNote, SIGNAL(triggered()), this, SLOT(removeNote()));

     menu->addAction(addNewF);
     menu->addAction(renameF);
     menu->addAction(removeFolder);
     menu->addSeparator();
     menu->addAction(addNewN);
     menu->addAction(renameN);
     menu->addAction(removeNote);
    
     menu->exec(globalPos);
}
