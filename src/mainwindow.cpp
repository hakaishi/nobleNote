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

     folderModel = new FileSystemModel(this);
     folderModel->setRootPath(origPath);
     folderModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);


     noteModel = new FileSystemModel(this);
     noteModel->setRootPath(origPath); //just as an example
     noteModel->setFilter(QDir::Files);

     folderList = new QListView(splitter); //note book list
     folderList->setModel(folderModel);
     folderList->setRootIndex(folderModel->index(origPath));
     folderList->setContextMenuPolicy(Qt::CustomContextMenu);
     //nbList->setEditTriggers(QListView::NoEditTriggers);
     noteList = new QListView(splitter);  //note list
     noteList->setEditTriggers(QListView::NoEditTriggers);
     noteList->setModel(noteModel);
     noteList->setRootIndex(noteModel->index(origPath));
     noteList->setContextMenuPolicy(Qt::CustomContextMenu);

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
     connect(folderList, SIGNAL(clicked(const QModelIndex &)), this, SLOT(setCurrentFolder(const QModelIndex &)));
     connect(folderList,SIGNAL(activated(QModelIndex)),this,SLOT(setCurrentFolder(QModelIndex)));
     connect(noteList, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(openNote(const QModelIndex &)));
     connect(noteList,SIGNAL(activated(QModelIndex)),this,SLOT(openNote(QModelIndex)));
     connect(folderList, SIGNAL(customContextMenuRequested(const QPoint &)),
    this, SLOT(showContextMenuF(const QPoint &)));
     connect(noteList, SIGNAL(customContextMenuRequested(const QPoint &)),
    this, SLOT(showContextMenuN(const QPoint &)));
}

NobleNote::~NobleNote(){}

void NobleNote::setCurrentFolder(const QModelIndex &ind){
     noteList->setRootIndex(noteModel->setRootPath(folderModel->filePath(ind)));
}

void NobleNote::openNote(const QModelIndex &index /* = new QModelIndex*/){
    QModelIndex ind = index;
    if(!ind.isValid()) // default constructed model index
        ind = noteList->currentIndex();

     QString newPath = noteModel->filePath(ind);
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

void NobleNote::newFolder(){
     QString path = origPath + "/" + tr("new folder");
     int counter = 0;
     while(QDir(path).exists())
     {
         ++counter;
         path = origPath + "/" + tr("new folder (") + QString::number(counter) + ")";
     }
     QDir().mkdir(path);
}

void NobleNote::newNote(){
    QString name = noteModel->rootPath() + "/" + tr("new note");
    int counter = 0;
    while(QFile::exists(name))
    {
        ++counter;
        name = noteModel->rootPath() + "/" + tr("new note (") + QString::number(counter) + ")";
    }

     QFile file(name);
     if(!file.open(QIODevice::WriteOnly))
       return;
     file.close();
}

void NobleNote::renameFolder(){
    folderList->edit(folderList->currentIndex());
}

void NobleNote::renameNote(){
    noteList->edit(noteList->currentIndex());
}

void NobleNote::removeFolder(){
     folderModel->rmdir(folderList->currentIndex());
//TODO: check why:
//QInotifyFileSystemWatcherEngine::addPaths: inotify_add_watch failed: Datei oder Verzeichnis nicht gefunden
//QFileSystemWatcher: failed to add paths: /home/hakaishi/.nobleNote/new folder
}

void NobleNote::removeNote(){
     noteModel->remove(noteList->currentIndex());
}

void NobleNote::showContextMenuF(const QPoint &pos){
     QPoint globalPos = this->mapToGlobal(pos);

     QMenu* menu = new QMenu(this);
     QAction* addNewF = new QAction(tr("New &folder"), this);
     QAction* renameF = new QAction(tr("R&ename folder"), this);
     QAction* removeFolder = new QAction(tr("&Remove folder"), this);

     connect(addNewF, SIGNAL(triggered()), this, SLOT(newFolder()));
     connect(renameF, SIGNAL(triggered()), this, SLOT(renameFolder()));
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

     connect(open, SIGNAL(triggered()), this, SLOT(openNote()));
     connect(addNewN, SIGNAL(triggered()), this, SLOT(newNote()));
     connect(renameN, SIGNAL(triggered()), this, SLOT(renameNote()));
     connect(removeNote, SIGNAL(triggered()), this, SLOT(removeNote()));

     menu->addAction(open);
     menu->addSeparator();
     menu->addAction(addNewN);
     menu->addAction(renameN);
     menu->addAction(removeNote);
    
     menu->exec(globalPos);
}
