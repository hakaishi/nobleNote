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

   //TrayIcon
     QIcon icon = QIcon(":nobleNote");
     TIcon = new QSystemTrayIcon(this);
     TIcon->setIcon(icon);
     TIcon->show();

   //TrayIconContextMenu
     iMenu = new QMenu(this);
     minimize_restore_action = new QAction(tr("&Minimize"),this);
     quit_action = new QAction(tr("&Quit"),this);

     iMenu->addAction(minimize_restore_action);
     iMenu->addAction(quit_action);

     TIcon->setContextMenu(iMenu);  //setting contextmenu for the systray

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

     folderList = new QListView(splitter);
     folderList->setModel(folderModel);
     folderList->setRootIndex(folderModel->index(origPath));
     folderList->setContextMenuPolicy(Qt::CustomContextMenu);
     noteList = new QListView(splitter);
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

     connect(actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
     connect(TIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
       this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason))); //handles systray-symbol
     connect(minimize_restore_action, SIGNAL(triggered()), this, SLOT(tray_actions()));
     connect(quit_action, SIGNAL(triggered()), qApp, SLOT(quit())); //contextmenu "Quit" for the systray
     connect(folderList, SIGNAL(clicked(const QModelIndex &)), this, SLOT(setCurrentFolder(const QModelIndex &)));
     connect(folderList,SIGNAL(activated(QModelIndex)),this,SLOT(setCurrentFolder(QModelIndex)));
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

void NobleNote::iconActivated(QSystemTrayIcon::ActivationReason reason){
     if(reason == QSystemTrayIcon::Trigger){
       if(isMinimized() || isHidden())  //in case that the window is minimized or hidden
         tray_actions();
     }
}

void NobleNote::tray_actions(){
     if(isMinimized() || isHidden())  //in case that the window is minimized or hidden
       showNormal();
     else
       hide();
}

void NobleNote::showEvent(QShowEvent* show_window){
     minimize_restore_action->setText(tr("&Minimize"));
     QWidget::showEvent(show_window);
}

void NobleNote::hideEvent(QHideEvent* window_hide){
     minimize_restore_action->setText(tr("&Restore"));
     QWidget::hideEvent(window_hide);
}

void NobleNote::closeEvent(QCloseEvent* window_close){
     //if(!pref->getQuitOnClose())
       hide();
     //else
       //qApp->quit();
     QWidget::closeEvent(window_close);
}

void NobleNote::keyPressEvent(QKeyEvent *kEvent){
     if(kEvent->modifiers() == Qt::ControlModifier)
       if(kEvent->key() == Qt::Key_Q)
         qApp->quit();
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
     notes->setAttribute(Qt::WA_DeleteOnClose);
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

     QMenu menu;
     QAction* addNewF = new QAction(tr("New &folder"), &menu);
     QAction* renameF = new QAction(tr("R&ename folder"), &menu);
     QAction* removeFolder = new QAction(tr("&Remove folder"), &menu);

     connect(addNewF, SIGNAL(triggered()), this, SLOT(newFolder()));
     connect(renameF, SIGNAL(triggered()), this, SLOT(renameFolder()));
     connect(removeFolder, SIGNAL(triggered()), this, SLOT(removeFolder()));

     menu.addAction(addNewF);
     menu.addAction(renameF);
     menu.addAction(removeFolder);
    
     menu.exec(globalPos);
}

void NobleNote::showContextMenuN(const QPoint &pos){
     QPoint globalPos = this->mapToGlobal(pos);

     QMenu menu;
     QAction* addNewN = new QAction(tr("New &note"), &menu);
     QAction* renameN = new QAction(tr("Ren&ame note"), &menu);
     QAction* removeNote = new QAction(tr("Re&move note"), &menu);

     connect(addNewN, SIGNAL(triggered()), this, SLOT(newNote()));
     connect(renameN, SIGNAL(triggered()), this, SLOT(renameNote()));
     connect(removeNote, SIGNAL(triggered()), this, SLOT(removeNote()));

     menu.addAction(addNewN);
     menu.addAction(renameN);
     menu.addAction(removeNote);
    
     menu.exec(globalPos);
}
