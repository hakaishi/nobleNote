/* nobleNote, a note taking application
 * Copyright (C) 2012 Christian Metscher <hakaishi@web.de>,
                      Fabian Deuchler <Taiko000@gmail.com>

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.

 * nobleNote is licensed under the MIT, see `http://copyfree.org/licenses/mit/license.txt'.
 */

#include "mainwindow.h"
#include "welcome.h"
#include "mainwindowtoolbar.h"
#include "note.h"
#include "findfilemodel.h"
#include "filesystemmodel.h"
#include "preferences.h"
#include "lineedit.h"
#include "findfilesystemmodel.h"
#include "highlighter.h"
#include "notedescriptor.h"
#include "htmlnotewriter.h"
#include <QTextStream>
#include <QFile>
#include <QModelIndex>
#include <QInputDialog>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QSettings>
#include <QMessageBox>
#include <QFileIconProvider>
#include <QList>
#include <QFileDialog>
#include <QPushButton>

NobleNote::NobleNote()
{
     setupUi(this);

   //TrayIcon
     QIcon icon = QIcon(":nobleNote");

     minimize_restore_action = new QAction(tr("&Minimize"),this);
     quit_action = new QAction(tr("&Quit"),this);

#ifndef NO_SYSTEM_TRAY_ICON
     TIcon = new QSystemTrayIcon(this);
     TIcon->setIcon(icon);
     TIcon->show();

   //TrayIconContextMenu
     iMenu = new QMenu(this);
     iMenu->addAction(minimize_restore_action);
     iMenu->addAction(quit_action);

     TIcon->setContextMenu(iMenu);  //setting contextmenu for the systray
#endif

   //Toolbar
     toolbar = new MainWindowToolbar(this);
     addToolBar(toolbar);

   //Configuration file
     QSettings settings; // ini format does save but in the executables directory, use native format
     if(!settings.isWritable()){
       QMessageBox::critical(this,tr("Settings not writable"),tr("W: nobelNote settings not writable!"));
     }
     if(!settings.value("rootPath").isValid()){ // root path has not been set before
       welcome = new Welcome(this);
       welcome->exec();
     }
     if(!settings.value("noteDirPath").isValid())
       settings.setValue("noteDirPath",settings.value("rootPath").toString() + "/notes");
     if(!settings.value("backupDirPath").isValid())
       settings.setValue("backupDirPath",settings.value("rootPath").toString() + "/backups");

     if(!QDir(settings.value("noteDirPath").toString()).exists())
       QDir().mkpath(settings.value("noteDirPath").toString());

     if(!QDir(settings.value("backupDirPath").toString()).exists())
       QDir().mkpath(settings.value("backupDirPath").toString());

   //Setup preferences
     pref = new Preferences(this);

   //Collabsable search
     hBoxLayout = new QHBoxLayout();
     gridLayout->addLayout(hBoxLayout, 0, 0);

   //Search line edits
     searchName = new LineEdit(this);
     searchName->setPlaceholderText(tr("Search for note"));
     gridLayout->addWidget(searchName, 1, 0);

     searchText = new LineEdit(this);
     searchText->setPlaceholderText(tr("Search for content"));
     gridLayout->addWidget(searchText, 2, 0);


     splitter = new QSplitter(centralwidget);
     gridLayout->addWidget(splitter, 3, 0);

     folderModel = new FileSystemModel(this);
     folderModel->setRootPath(settings.value("noteDirPath").toString());
     folderModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
     folderModel->setReadOnly(false);
     folderModel->setOnlyOnItemDrops(true);

     noteFSModel = new FileSystemModel(this);
     noteFSModel->setFilter(QDir::Files);
     noteFSModel->setReadOnly(false);

     //foreach(QString str, noteFSModel->mimeTypes())
     //    qDebug() << "mime types: " << str;

     findNoteModel = new FindFileModel(this);

     noteModel = new FindFileSystemModel(this);
     noteModel->setSourceModel(noteFSModel);

     folderList = new QListView(splitter);
     noteList = new QListView(splitter);

     QList<QListView*> listViews;
     listViews << folderList << noteList;
     foreach(QListView* list, listViews) // add drag drop options
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
     folderList->setRootIndex(folderModel->index(settings.value("noteDirPath").toString()));
     noteList->setEditTriggers(QListView::EditKeyPressed);
     noteList->setModel(noteModel);

     // make sure there's at least one folder
     QStringList dirList = QDir(settings.value("noteDirPath").toString()).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
     if(dirList.isEmpty())
     {
         QString defaultDirName = tr("default");
         QDir(settings.value("noteDirPath").toString()).mkdir(defaultDirName);
         noteList->setRootIndex(noteModel->setRootPath(settings.value("noteDirPath").toString() + "/" + defaultDirName)); // set default dir as current note folder
     }
     else
         noteList->setRootIndex(noteModel->setRootPath(settings.value("noteDirPath").toString() + "/" + dirList.first())); // dirs exist, set first dir as current note folder

//TODO: make it possible to import notes from some other folder or even another program

//     // selects first folder as soon as the folderModel has populated its first folder
//     // "single shot" slot
     connect(folderModel,SIGNAL(directoryLoaded(QString)), this,
       SLOT(selectFirstFolder(QString)),Qt::QueuedConnection);

     connect(searchName, SIGNAL(textChanged(const QString)), this, SLOT(find()));
     connect(searchText, SIGNAL(textChanged(const QString)), this, SLOT(find()));
     connect(folderList->itemDelegate(),SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),this,SLOT(folderRenameFinished(QWidget*,QAbstractItemDelegate::EndEditHint)));
     //connect(noteList->itemDelegate(),SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),this,SLOT(noteRenameFinished(QWidget*,QAbstractItemDelegate::EndEditHint)));
     connect(noteFSModel,SIGNAL(fileRenamed(QString,QString,QString)),this,SLOT(noteRenameFinished(QString,QString,QString)));
     connect(action_Import,SIGNAL(triggered()),this,SLOT(importXmlNotes()));
     connect(actionQuit, SIGNAL(triggered()), this, SLOT(quit()));
    #ifndef NO_SYSTEM_TRAY_ICON
     connect(TIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
       this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason))); //handles systray-symbol     
     connect(minimize_restore_action, SIGNAL(triggered()), this, SLOT(tray_actions()));
      #endif
     connect(quit_action, SIGNAL(triggered()), this, SLOT(quit())); //contextmenu "Quit" for the systray
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
     connect(actionAbout,SIGNAL(triggered()),this,SLOT(about()));
     connect(action_Show_toolbar, SIGNAL(toggled(bool)), toolbar, SLOT(setVisible(bool)));
     connect(toolbar->newFolderAction, SIGNAL(triggered()), this, SLOT(newFolder()));
     connect(toolbar->newNoteAction, SIGNAL(triggered()), this, SLOT(newNote()));
}

NobleNote::~NobleNote(){}

void NobleNote::find(){
         noteModel->setSourceModel(findNoteModel);
         noteModel->clear(); // if findNoteModel already set, clear old found list
         noteModel->findInFiles(searchName->text(),searchText->text(),folderModel->rootPath());
}

void NobleNote::selectFirstFolder(QString path)
{
    Q_UNUSED(path);
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

     thisMethodHasBeenCalled = true;
}

void NobleNote::folderRenameFinished(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    Q_UNUSED(editor);
    if(hint != QAbstractItemDelegate::RevertModelCache) // canceled editing
        setCurrentFolder(folderList->currentIndex());
}

void NobleNote::noteRenameFinished(const QString & path, const QString & oldName, const QString & newName)
{

    QString filePath = noteModel->filePath(noteList->currentIndex());
    Note * w = noteWindow(path + "/" + oldName);
    if(w)
        w->setWindowTitle(QFileInfo(path + "/" + newName).baseName());
}

void NobleNote::setCurrentFolder(const QModelIndex &ind){
    // clear search line edits
    searchName->clear();
    searchText->clear();
     noteModel->setSourceModel(noteFSModel);
     noteList->setRootIndex(noteModel->setRootPath(folderModel->filePath(ind)));
}

void NobleNote::changeRootIndex(){
    if(!openNotes.isEmpty()){
        foreach(Note *note, openNotes)
            if(note)
                note->close();
        openNotes.clear();
    }
     QStringList dirList = QDir(QSettings().value("noteDirPath").toString()).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
     noteList->setRootIndex(noteModel->setRootPath(QSettings().value("noteDirPath").toString() +
                                                   "/" + dirList.first()));
     folderList->setRootIndex(folderModel->index(QSettings().value("noteDirPath").toString()));
}

#ifndef NO_SYSTEM_TRAY_ICON
void NobleNote::iconActivated(QSystemTrayIcon::ActivationReason reason){
     if(reason == QSystemTrayIcon::Trigger)
       tray_actions();
}
#endif

#ifndef NO_SYSTEM_TRAY_ICON
void NobleNote::tray_actions(){
     if(isMinimized() || isHidden())  //in case that the window is minimized or hidden
       showNormal();
     else
       hide();
}
#endif

void NobleNote::showEvent(QShowEvent* show_window){

     if(QSettings().contains("mainwindow_size"))
       restoreGeometry(QSettings().value("mainwindow_size").toByteArray());
     QMainWindow::showEvent(show_window);
}

void NobleNote::hideEvent(QHideEvent* window_hide){
     minimize_restore_action->setText(tr("&Restore"));
     QMainWindow::hideEvent(window_hide);
}

void NobleNote::closeEvent(QCloseEvent* window_close){
     if(pref->dontQuit->isChecked())
     {
       hide();
     }
     else{
       QSettings().setValue("mainwindow_size", saveGeometry());
       qApp->quit();
     }
     QMainWindow::closeEvent(window_close);
}

void NobleNote::quit()
{
    QSettings().setValue("mainwindow_size", saveGeometry());
    qApp->quit();
}

void NobleNote::openNote(const QModelIndex &index /* = new QModelIndex*/){
     QModelIndex ind = index;
     if(!ind.isValid()) // default constructed model index
       ind = noteList->currentIndex();

     QString notePath = noteModel->filePath(ind);
     if(!QFileInfo(notePath).exists())
     {
         QMessageBox::warning(this,tr("Note does not exist"), tr("The selected note cannot be opened because it has been moved or renamed"));
         return;
     }

      // check if the notePath is already used in a open note
     Note* w = noteWindow(notePath);
     if(w)
     {
         w->activateWindow();  // highlight the note window
         return;
     }

     Note* note=new Note(notePath);
     openNotes+= note;
     note->setObjectName(notePath);
     if(noteModel->sourceModel() == findNoteModel){
       note->highlightText(searchText->text());
       note->searchbarVisible = true;
       note->setSearchBarText(searchText->text());
     }
     note->show();
}

Note *NobleNote::noteWindow(const QString &filePath)
{
    for(QList<QPointer<Note> >::Iterator it = openNotes.begin(); it < openNotes.end(); ++it)
    {
        // remove NULL pointers, if the Note widget is destroyed, its pointer is automatically set to null
        if(!(*it))
        {
            it = openNotes.erase(it); // skip iterator position
            continue;
        }

        if((*it) && (*it)->noteDescriptor()->filePath() == filePath)
        {
            return (*it);
        }
    }
    return 0;
}

void NobleNote::newFolder(){
     QString path = folderModel->rootPath() + "/" + tr("new folder");
     int counter = 0;
     while(QDir(path).exists())
     {
         ++counter;
         path = folderModel->rootPath() + "/" + tr("new folder (%1)").arg(QString::number(counter));
     }
     folderModel->mkdir(folderList->rootIndex(),QDir(path).dirName());
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

     Note* note=new Note(name);
     openNotes+= note;
     note->setObjectName(name);
     note->show();
}

void NobleNote::renameFolder(){
     folderList->edit(folderList->currentIndex());
}

void NobleNote::renameNote(){
     noteList->edit(noteList->currentIndex());
}

void NobleNote::removeFolder(){

     QStringList dirList = QDir(QSettings().value("noteDirPath").toString()).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

     // only needed if journal folder resides in the same folder
     //dirList.removeOne(QDir(QSettings().value("journalFolderPath").toString()).dirName());

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
         if(QMessageBox::warning(this,tr("Delete Folder"),
                                tr("Do you really want to delete the notebook \"%1\"? All contained notes will be lost?").arg(folderModel->fileName(idx)),
                             QMessageBox::Yes | QMessageBox::Abort) != QMessageBox::Yes)
            return;


        // list all files
        QString path = folderModel->filePath(idx);
        QStringList fileList = QDir(path).entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

        // try to remove each
        foreach(QString name, fileList)
        {
            name = QDir::toNativeSeparators(QString("%1/%2").arg(path).arg(name));
            if(!folderModel->remove(folderModel->index(name)))
            {
                qWarning(qPrintable(QString("could not delete ") + name));
            }
        }

        // try to remove the (now empty?) folder again
        if(!folderModel->rmdir(idx))
        {
            QMessageBox::warning(this,tr("Folder could not be deleted"), tr("The folder could not be deleted because one or more files inside the folder could not be deleted"));
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
     if(QMessageBox::warning(this,tr("Delete note"),
         tr("Do you really want to delete the note \"%1\"?").arg(noteModel->fileName(noteList->currentIndex())),
           QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
       return;
     noteModel->remove(noteList->currentIndex());
}

void NobleNote::importXmlNotes()
{
    QStringList files = QFileDialog::getOpenFileNames(
                            this,
                            tr("Select one or more files to open"),
                            "/home",
                tr("Notes")+"(*.note)");
    if(files.isEmpty())
        return;

    // TODO save last selected path

    foreach(QString filePath, files)
    {
        HtmlNoteWriter::writeXml2Html(filePath,QSettings().value("noteDirPath").toString());
    }
}

void NobleNote::showContextMenuF(const QPoint &pos){
     QPoint globalPos = folderList->mapToGlobal(pos);

     QMenu menu;

     if(!folderList->indexAt(pos).isValid()) // if index doesn't exists at position
     {
         QAction* addNewF = new QAction(tr("&New folder"), &menu);
         connect(addNewF, SIGNAL(triggered()), this, SLOT(newFolder()));
         menu.addAction(addNewF);
     }
     if(folderList->indexAt(pos).isValid()) // if index exists at position
     {
         QAction* renameF = new QAction(tr("&Rename folder"), &menu);
         QAction* removeFolder = new QAction(tr("&Delete folder"), &menu);
         connect(renameF, SIGNAL(triggered()), this, SLOT(renameFolder()));
         connect(removeFolder, SIGNAL(triggered()), this, SLOT(removeFolder()));
         menu.addAction(renameF);
         menu.addAction(removeFolder);
     }
     menu.exec(globalPos);
}

void NobleNote::showContextMenuN(const QPoint &pos){
     QPoint globalPos = noteList->mapToGlobal(pos);

     QMenu menu;

     if(!noteList->indexAt(pos).isValid() &&
        !(noteModel->sourceModel() == findNoteModel)) // if index doesn't exists at position
     {
         QAction* addNewN = new QAction(tr("&New note"), &menu);
         connect(addNewN, SIGNAL(triggered()), this, SLOT(newNote()));
         menu.addAction(addNewN);
     }
     if(noteList->indexAt(pos).isValid()) // if index exists at position
     {
         QAction* renameN = new QAction(tr("&Rename note"), &menu);
         QAction* removeNote = new QAction(tr("&Delete note"), &menu);
         connect(renameN, SIGNAL(triggered()), this, SLOT(renameNote()));
         connect(removeNote, SIGNAL(triggered()), this, SLOT(removeNote()));
         menu.addAction(renameN);
         menu.addAction(removeNote);
     }
     menu.exec(globalPos);
}

void NobleNote::keyPressEvent(QKeyEvent *k){
     if(k->key() == Qt::Key_Delete){
       if(noteList->hasFocus())
         removeNote();
       if(folderList->hasFocus())
         removeFolder();
     }
}

void NobleNote::about()
{
   QMessageBox::about(this, tr("About ") + QApplication::applicationName(),
                      tr("<p><b>%1</b> is a note taking application</p>"
                   "<p>Copyright (C) 2012 Christian Metscher, Fabian Deuchler</p>"

                   "<p>Permission is hereby granted, free of charge,"
                   " to any person obtaining a copy of this software and associated documentation files (the \"Software\"),"
                   "to deal in the Software without restriction,"
                   "including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,"
                   "and to permit persons to whom the Software is furnished to do so, subject to the following conditions:</p>"
                   "The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software."
                   "<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,"
                   "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,"
                   "WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</p>"
                              ).arg(QApplication::applicationName())); //: %1 is the application name, also do not translate the licence text
}
