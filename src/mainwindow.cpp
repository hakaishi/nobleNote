#include "mainwindow.h"
#include "note.h"
#include "filesystemmodel.h"
#include "preferences.h"
#include <QTextStream>
#include <QFile>
#include <QModelIndex>
#include <QInputDialog>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QtConcurrentRun>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>

NobleNote::NobleNote() : journalFolderName("Journals")
{
     setupUi(this);

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

     QSettings settings;
     QSettings::setDefaultFormat(QSettings::IniFormat);
     if(!settings.isWritable()){
       qWarning("W: nobelNote settings not writable!");
     }

     origPath = settings.value("Path to note folders",QDir::homePath() + "/.nobleNote").toString();
     pref = new Preferences(this);
     pref->lineEdit->setText(origPath);
     pref->pSpin->setValue(settings.value("Save notes periodically",0).toInt());

     QDir nbDir(QDir::homePath() + "/.nobleNote/" + journalFolderName);
     if(!nbDir.exists())
       nbDir.mkdir(QDir::homePath() + "/.nobleNote/" + journalFolderName);

     // make sure there's at least one folder
     QStringList dirList = nbDir.entryList(QDir::NoDotAndDotDot);
     dirList.removeOne(journalFolderName);
     if(dirList.isEmpty())
     {
         QDir(origPath).mkdir(tr("default"));
     }


     splitter = new QSplitter(centralwidget);
     gridLayout->addWidget(splitter, 0, 0);

     folderModel = new FileSystemModel(this);
     folderModel->setRootPath(origPath);
     folderModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
     folderModel->setReadOnly(false); // enable drag drop
     //folderModel->setNameFilters(QStringList("Journals")); TODO:DON'T show journals.

     noteModel = new FileSystemModel(this);
     noteModel->setRootPath(origPath); //just as an example
     noteModel->setFilter(QDir::Files);
     noteModel->setReadOnly(false);

     folderList = new QListView(splitter);
     noteList = new QListView(splitter);

     QList<QListView*> listViews;
     listViews << folderList << noteList;
     foreach(QListView* list, listViews)
     {
        list->setContextMenuPolicy(Qt::CustomContextMenu);
        //list->setSelectionMode(QAbstractItemView::SingleSelection); // single item can be draged or droped

        list->setDragDropMode(QAbstractItemView::DragDrop);
        list->viewport()->setAcceptDrops(true);
        list->setDropIndicatorShown(true);
        list->setDefaultDropAction(Qt::MoveAction);
     }
     noteList->setDragEnabled(true);
     folderList->setDragEnabled(false);

     folderList->setModel(folderModel);
     folderList->setRootIndex(folderModel->index(origPath));
     noteList->setEditTriggers(QListView::EditKeyPressed);
     noteList->setModel(noteModel);
     noteList->setRootIndex(noteModel->index(origPath));

//TODO: make it possible to import notes from some other folder or even another program

     // sets random folder by default as current folder
     // "single shot" slot
     connect(folderModel,SIGNAL(directoryLoaded(QString)), this,
       SLOT(setFirstFolderCurrent(QString)),Qt::QueuedConnection);

     connect(actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
     connect(TIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
       this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason))); //handles systray-symbol
     connect(minimize_restore_action, SIGNAL(triggered()), this, SLOT(tray_actions()));
     connect(quit_action, SIGNAL(triggered()), qApp, SLOT(quit())); //contextmenu "Quit" for the systray
     connect(folderList, SIGNAL(clicked(const QModelIndex &)), this,
       SLOT(setCurrentFolder(const QModelIndex &)));
     connect(folderList,SIGNAL(activated(QModelIndex)), this,
       SLOT(setCurrentFolder(QModelIndex)));
     connect(noteList,SIGNAL(activated(QModelIndex)), this,
       SLOT(openNote(QModelIndex)));
     connect(folderList, SIGNAL(customContextMenuRequested(const QPoint &)),
       this, SLOT(showContextMenuF(const QPoint &)));
     connect(noteList, SIGNAL(customContextMenuRequested(const QPoint &)),
       this, SLOT(showContextMenuN(const QPoint &)));
     connect(action_Configure, SIGNAL(triggered()), pref, SLOT(show()));
     connect(pref, SIGNAL(sendPathChanged()), this, SLOT(changeRootIndex()));
}

NobleNote::~NobleNote(){}

void NobleNote::setFirstFolderCurrent(QString path)
{
    // this slot gets (probably) called by the QFileSystemModel gatherer thread
    // due to some race conditions:
    // disconnecting this slot will not work button disconnect() will return true
    // qDebug() may work or not work depending how many time has elapsed in this function

    // only call once
    static bool thisMethodHasBeenCalled = false;

    if(thisMethodHasBeenCalled)
        return;

    QModelIndex idx = folderList->indexAt(QPoint(0,0));
    if(!idx.isValid())
    return;

    folderList->selectionModel()->select(idx,QItemSelectionModel::Select);
    setCurrentFolder(idx);

    thisMethodHasBeenCalled = true;
}

void NobleNote::setCurrentFolder(const QModelIndex &ind){
     noteList->setRootIndex(noteModel->setRootPath(folderModel->filePath(ind)));
}

void NobleNote::changeRootIndex(){
     if(pref->lineEdit->text().isEmpty()){
       origPath = QDir::homePath() + "/.nobleNote";
       folderModel->setRootPath(origPath);
       noteModel->setRootPath(origPath);
       folderList->setRootIndex(folderModel->index(origPath));
       noteList->setRootIndex(noteModel->index(origPath));
     }
     else{
       folderModel->setRootPath(pref->lineEdit->text());
       noteModel->setRootPath(pref->lineEdit->text());
       folderList->setRootIndex(folderModel->index(pref->lineEdit->text()));
       noteList->setRootIndex(noteModel->index(pref->lineEdit->text()));
     }
}

void NobleNote::iconActivated(QSystemTrayIcon::ActivationReason reason){
     if(reason == QSystemTrayIcon::Trigger)
       tray_actions();
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

     QString notesPath = noteModel->filePath(ind);
     QFile noteFile(notesPath);
     if(!noteFile.open(QIODevice::ReadOnly))
       return;
     QTextStream streamN(&noteFile);
     text = streamN.readAll();
     noteFile.close();

     QString journalsPath = QDir::homePath() + "/.nobleNote/Journals/" +
       folderModel->fileName(folderList->currentIndex()) + "_" + noteModel->fileName(ind) + ".journal";
     QFile journalFile(journalsPath);
     if(!journalFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
       return;
     if(!journalFile.exists()){
       QTextStream streamJ(&journalFile);
       streamJ << text;
     }
     journalFile.close();

     //TODO:
     //if(QFileInfo(journalsPath).lastModified().toString() == QFileInfo(notesPath).lastModified().toString());

     Note *notes = new Note(this);
     notes->text = text;
     notes->notesPath = notesPath;
     notes->journalsPath = journalsPath;
     if(pref->pSpin->value() > 0)
       notes->timer->start(pref->pSpin->value() * 60000);
     notes->show();
     notes->setAttribute(Qt::WA_DeleteOnClose);
}

void NobleNote::newFolder(){
     QString path = folderModel->rootPath() + "/" + tr("new folder");
     int counter = 0;
     while(QDir(path).exists())
     {
         ++counter;
         path = folderModel->rootPath() + "/" + tr("new folder (%1)").arg(QString::number(counter));
     }
     QDir().mkdir(path);
}

void NobleNote::newNote(){
    QString name = noteModel->rootPath() + "/" + tr("new note");
    int counter = 0;
    while(QFile::exists(name))
    {
        ++counter;
        name = noteModel->rootPath() + "/" + tr("new note (%1)").arg(QString::number(counter));
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

static void recurseAddDir(QDir d, QStringList & list) {

    QStringList qsl = d.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

    foreach (QString file, qsl) {

        QFileInfo finfo(QString("%1/%2").arg(d.path()).arg(file));

        if (finfo.isSymLink())
        {
            qDebug("symlink found");
            return;
        }

        if (finfo.isDir()) {

            QString dirname = finfo.fileName();
            QDir sd(finfo.filePath());

            recurseAddDir(sd, list);

        } else
            list << QDir::toNativeSeparators(finfo.filePath());
    }
}

void NobleNote::removeFolder(){

    QStringList dirList = QDir(origPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    dirList.removeOne(journalFolderName);

    // keep at least one folder
    if(dirList.size() == 1)
    {
        QMessageBox::information(this,tr("One notebook must remain"),tr("At least one notebook must remain."));
        return;
    }

    QModelIndex idx = folderList->currentIndex();

    // remove empty folders without prompt else show a yes/abort message box
    if(!folderModel->rmdir(idx)) // folder not empty
    {
        if(QMessageBox::warning(this,tr("Remove Folder"),
                                tr("Do you really want to delete the notebook \"%1\"? All contained notes will be lost?").arg(folderModel->fileName(idx)),
                             QMessageBox::Yes | QMessageBox::Abort) != QMessageBox::Yes)
            return;


        // list all files & folders recursively
        QString path = folderModel->filePath(idx);
        QStringList fileList = QDir(path).entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

        // try to remove each
        foreach(QString name, fileList)
        {
            name = QDir::toNativeSeparators(QString("%1/%2").arg(path).arg(name));
            if(!QFile::remove(name))
            {
                qWarning(qPrintable(QString("could not remove ") + name));
            }
        }

        // try to remove the (now empty?) folder again
        if(!folderModel->rmdir(idx))
        {
            QMessageBox::warning(this,tr("Folder could not be removed"), tr("The folder could not be removed because one or more files inside the folder could not be removed"));
            return;
        }
    }

// // TODO  Important! the following #ifdef code must only be executed if the folder has been removed
#ifdef Q_OS_WIN32
    // gives error QFileSystemWatcher: FindNextChangeNotification failed!! (Zugriff verweigert)
    // and dir deletion is delayed until another dir has been selected or the application is closed

    folderList->setRowHidden(idx.row(),true);
    QModelIndex idxAt = folderList->indexAt(QPoint(0,0));
    if(!idxAt.isValid())
    return;

    folderList->selectionModel()->select(idxAt,QItemSelectionModel::Select);
    setCurrentFolder(idxAt);
#endif

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
     connect(addNewF, SIGNAL(triggered()), this, SLOT(newFolder()));
     menu.addAction(addNewF);

     if(folderList->indexAt(pos).isValid()) // if index exists at position
     {
         QAction* renameF = new QAction(tr("R&ename folder"), &menu);
         QAction* removeFolder = new QAction(tr("&Remove folder"), &menu);
         connect(renameF, SIGNAL(triggered()), this, SLOT(renameFolder()));
         connect(removeFolder, SIGNAL(triggered()), this, SLOT(removeFolder()));
         menu.addAction(renameF);
         menu.addAction(removeFolder);
     }
     menu.exec(globalPos);
}

void NobleNote::showContextMenuN(const QPoint &pos){
     QPoint globalPos = this->mapToGlobal(pos);

     QMenu menu;
     QAction* addNewN = new QAction(tr("New &note"), &menu);
     connect(addNewN, SIGNAL(triggered()), this, SLOT(newNote()));
     menu.addAction(addNewN);

     if(noteList->indexAt(pos).isValid()) // if index exists at position
     {
         QAction* renameN = new QAction(tr("Ren&ame note"), &menu);
         QAction* removeNote = new QAction(tr("Re&move note"), &menu);
         connect(renameN, SIGNAL(triggered()), this, SLOT(renameNote()));
         connect(removeNote, SIGNAL(triggered()), this, SLOT(removeNote()));
         menu.addAction(renameN);
         menu.addAction(removeNote);
     }
     menu.exec(globalPos);
}
