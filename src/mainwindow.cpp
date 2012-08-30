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
#include "htmlnotereader.h"
#include "fileiconprovider.h"
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

MainWindow::MainWindow()
{
     setupUi(this);

   //TrayIcon
     QIcon icon = QIcon(":nobleNote");

     minimizeRestoreAction = new QAction(tr("&Minimize"),this);
     quit_action = new QAction(tr("&Quit"),this);

#ifndef NO_SYSTEM_TRAY_ICON
     TIcon = new QSystemTrayIcon(this);
     TIcon->setIcon(icon);
     TIcon->show();

   //TrayIconContextMenu
     iMenu = new QMenu(this);
     iMenu->addAction(minimizeRestoreAction);
     iMenu->addAction(quit_action);

     TIcon->setContextMenu(iMenu);  //setting contextmenu for the systray
#endif

   //Toolbar
     toolbar = new MainWindowToolbar(this);
     addToolBar(toolbar);
     actionShowToolbar->setChecked(QSettings().value("mainwindow_toolbar_visible", true).toBool());
     toolbar->setVisible(QSettings().value("mainwindow_toolbar_visible", true).toBool());

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

   //Setup preferences
     pref = new Preferences(this);

   //Search line edits
     searchName = new LineEdit(this);
     searchName->setPlaceholderText(tr("Search for note"));
     gridLayout->addWidget(searchName, 1, 0);

     searchText = new LineEdit(this);
     searchText->setPlaceholderText(tr("Search for content"));
     gridLayout->addWidget(searchText, 2, 0);


     splitter = new QSplitter(centralwidget);
     gridLayout->addWidget(splitter, 3, 0);


     folderModel = new FindFileSystemModel(this);
     FileSystemModel *folderFSModel = new FileSystemModel(this);
     folderFSModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
     folderFSModel->setReadOnly(false);
     folderFSModel->setOnlyOnItemDrops(true);
     folderFSModel->setIconProvider(new FileIconProvider());
     folderModel->setSourceModel(folderFSModel);

     noteFSModel = new FileSystemModel(this);
     noteFSModel->setFilter(QDir::Files);
     noteFSModel->setReadOnly(false);

     //foreach(QString str, noteFSModel->mimeTypes())
     //    qDebug() << "mime types: " << str;

     findNoteModel = new FindFileModel(this);

     noteModel = new FindFileSystemModel(this);
     noteModel->setSourceModel(noteFSModel);

     folderView = new QListView(splitter);
     noteView = new QListView(splitter);

     QList<QListView*> listViews;
     listViews << folderView << noteView;
     foreach(QListView* list, listViews) // add drag drop options
     {
        list->setContextMenuPolicy(Qt::CustomContextMenu);
        //list->setSelectionMode(QAbstractItemView::SingleSelection); // single item can be draged or droped
        list->setDragDropMode(QAbstractItemView::DragDrop);
        list->viewport()->setAcceptDrops(true);
        list->setDropIndicatorShown(true);
        list->setDefaultDropAction(Qt::MoveAction);
     }
     noteView->setDragEnabled(true);
     folderView->setDragEnabled(false);

     folderView->setModel(folderModel);
     noteView->setEditTriggers(QListView::EditKeyPressed);
     noteView->setModel(noteModel);

     checkAndSetFolders();

//TODO: make it possible to import notes from some other folder or even another program

//     // selects first folder as soon as the folderModel has populated its first folder
//     // "single shot" slot
     connect(folderFSModel,SIGNAL(directoryLoaded(QString)), this,
             SLOT(selectFirstFolder(QString)),Qt::QueuedConnection);
     connect(folderView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this,SLOT(onFolderSelectionChanged(QItemSelection,QItemSelection)));
     connect(searchName, SIGNAL(textChanged(const QString)), this, SLOT(find()));
     connect(searchText, SIGNAL(textChanged(const QString)), this, SLOT(find()));
     connect(folderView->itemDelegate(),SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),this,SLOT(folderRenameFinished(QWidget*,QAbstractItemDelegate::EndEditHint)));
     //connect(noteList->itemDelegate(),SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),this,SLOT(noteRenameFinished(QWidget*,QAbstractItemDelegate::EndEditHint)));
     connect(noteFSModel,SIGNAL(fileRenamed(QString,QString,QString)),this,SLOT(noteRenameFinished(QString,QString,QString)));
     connect(actionImport,SIGNAL(triggered()),this,SLOT(importXmlNotes()));
     connect(actionQuit, SIGNAL(triggered()), this, SLOT(quit()));
#ifndef NO_SYSTEM_TRAY_ICON
     connect(TIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
             this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason))); //handles systray-symbol
     connect(minimizeRestoreAction, SIGNAL(triggered()), this, SLOT(tray_actions()));
#endif
     connect(quit_action, SIGNAL(triggered()), this, SLOT(quit())); //contextmenu "Quit" for the systray
     //     connect(folderList, SIGNAL(clicked(const QModelIndex &)), this,
     //       SLOT(setCurrentFolder(const QModelIndex &)));
     //     connect(folderList,SIGNAL(activated(QModelIndex)), this,
     //       SLOT(setCurrentFolder(QModelIndex)));
     connect(noteView,SIGNAL(activated(QModelIndex)), this,
             SLOT(openNote(QModelIndex)));
     connect(folderView, SIGNAL(customContextMenuRequested(const QPoint &)),
             this, SLOT(showContextMenuFolder(const QPoint &)));
     connect(noteView, SIGNAL(customContextMenuRequested(const QPoint &)),
             this, SLOT(showContextMenuNote(const QPoint &)));
     connect(actionConfigure, SIGNAL(triggered()), pref, SLOT(show()));
     connect(pref, SIGNAL(sendPathChanged()), this, SLOT(changeRootIndex()));
     connect(actionAbout,SIGNAL(triggered()),this,SLOT(about()));
     connect(actionShowToolbar, SIGNAL(toggled(bool)), toolbar, SLOT(setVisible(bool)));
     connect(toolbar, SIGNAL(visibilityChanged(bool)), actionShowToolbar, SLOT(setChecked(bool)));
     connect(toolbar->newFolderAction, SIGNAL(triggered()), this, SLOT(newFolder()));
     connect(toolbar->newNoteAction, SIGNAL(triggered()), this, SLOT(newNote()));
     connect(toolbar->removeFolderAction,SIGNAL(triggered()),this,SLOT(removeFolder()));
     connect(toolbar->removeNoteAction,SIGNAL(triggered()),this,SLOT(removeNote()));
     connect(toolbar->renameFolderAction,SIGNAL(triggered()),this,SLOT(renameFolder()));
     connect(toolbar->renameNoteAction,SIGNAL(triggered()),this,SLOT(renameNote()));
     connect(folderView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),toolbar,SLOT(onFolderSelectionChanged(QItemSelection,QItemSelection)));
     connect(noteView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),toolbar,SLOT(onNoteSelectionChanged(QItemSelection,QItemSelection)));
}

MainWindow::~MainWindow(){}

void MainWindow::find(){
         noteModel->setSourceModel(findNoteModel);
         noteModel->clear(); // if findNoteModel already set, clear old found list
         noteModel->findInFiles(searchName->text(),searchText->text(),folderModel->rootPath());

}

void MainWindow::selectFirstFolder(QString path)
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

     QModelIndex idx = folderView->indexAt(QPoint(0,0));
     if(!idx.isValid())
       return;

     folderView->setCurrentIndex(idx);
     //folderView->selectionModel()->select(idx,QItemSelectionModel::Select);

     thisMethodHasBeenCalled = true;
}

void MainWindow::folderRenameFinished(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    Q_UNUSED(editor);
     if(hint != QAbstractItemDelegate::RevertModelCache) // canceled editing
     {
         QString currFolderPath = folderModel->filePath(folderView->currentIndex());
         //folderModel->sort(1);// does not work with indices 0 -3
         folderView->setCurrentIndex(folderModel->index(currFolderPath));
     }


 // disable note toolbar buttons if selection is cleared after the folder has been renamed
     if(!noteView->selectionModel()->hasSelection())
        toolbar->onNoteSelectionChanged(QItemSelection(),QItemSelection());
}

void MainWindow::noteRenameFinished(const QString & path, const QString & oldName, const QString & newName)
{
     QString filePath = noteModel->filePath(noteView->currentIndex());
     Note * w = noteWindow(path + "/" + oldName);
     if(w)
        w->setWindowTitle(QFileInfo(path + "/" + newName).baseName());

     //noteView->model()->sort(1); // does not work
     noteView->setCurrentIndex(noteModel->index(filePath));
}

void MainWindow::setCurrentFolder(const QModelIndex &ind){
     // clear search line edits
     searchName->clear();
     searchText->clear();
     noteModel->setSourceModel(noteFSModel);
     noteView->setRootIndex(noteModel->setRootPath(folderModel->filePath(ind)));
     //noteList->setRootIndex(noteModel->index(folderModel->filePath(ind)));
}

void MainWindow::onFolderSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
     setCurrentFolder(selected.indexes().first());
     toolbar->onNoteSelectionChanged(QItemSelection(),QItemSelection()); // call the slot with an empty selection, this will disable the note toolbar buttons
}

void MainWindow::changeRootIndex(){
     if(!openNotes.isEmpty()){
        foreach(Note *note, openNotes)
            if(note)
                note->close();
        openNotes.clear();
     }
     checkAndSetFolders();
}

void MainWindow::checkAndSetFolders(){
     if(!QDir(QSettings().value("noteDirPath").toString()).exists())
       QDir().mkpath(QSettings().value("noteDirPath").toString());

     if(!QDir(QSettings().value("backupDirPath").toString()).exists())
       QDir().mkpath(QSettings().value("backupDirPath").toString());

     folderView->setRootIndex(folderModel->setRootPath(QSettings().value("noteDirPath").toString()));

     // make sure there's at least one folder
     QStringList dirList = QDir(QSettings().value("noteDirPath").toString()).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
     if(dirList.isEmpty())
     {
         QString defaultDirName = tr("default");
         QDir(QSettings().value("noteDirPath").toString()).mkdir(defaultDirName);
         noteView->setRootIndex(noteModel->setRootPath(QSettings().value("noteDirPath").toString() + "/" + defaultDirName)); // set default dir as current note folder
     }
     else
         noteView->setRootIndex(noteModel->setRootPath(QSettings().value("noteDirPath").toString() + "/" + dirList.first())); // dirs exist, set first dir as current note folder
}

#ifndef NO_SYSTEM_TRAY_ICON
void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason){
     if(reason == QSystemTrayIcon::Trigger)
       tray_actions();
}
#endif

#ifndef NO_SYSTEM_TRAY_ICON
void MainWindow::tray_actions(){
     if(isMinimized() || isHidden())  //in case that the window is minimized or hidden
       showNormal();
     else
       hide();
}
#endif

void MainWindow::showEvent(QShowEvent* show_window){

     if(QSettings().contains("mainwindow_size"))
       restoreGeometry(QSettings().value("mainwindow_size").toByteArray());
     if(QSettings().contains("Splitter"))
       splitter->restoreState(QSettings().value("Splitter").toByteArray());
     QMainWindow::showEvent(show_window);
}

void MainWindow::hideEvent(QHideEvent* window_hide){
     minimizeRestoreAction->setText(tr("&Restore"));
     QMainWindow::hideEvent(window_hide);
}

void MainWindow::closeEvent(QCloseEvent* window_close){
     if(pref->dontQuit->isChecked())
     {
       hide();
     }
     else{
       QSettings().setValue("mainwindow_size", saveGeometry());
       QSettings().setValue("Splitter", splitter->saveState());
       qApp->quit();
     }
     QMainWindow::closeEvent(window_close);
}

void MainWindow::quit()
{
     QSettings().setValue("mainwindow_size", saveGeometry());
     QSettings().setValue("mainwindow_toolbar_visible", actionShowToolbar->isChecked());
     qApp->quit();
}

void MainWindow::openNote(const QModelIndex &index /* = new QModelIndex*/){
     QModelIndex ind = index;
     if(!ind.isValid()) // default constructed model index
       ind = noteView->currentIndex();

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

Note *MainWindow::noteWindow(const QString &filePath)
{
     QString uuid = HtmlNoteReader::uuid(filePath);
     for(QList<QPointer<Note> >::Iterator it = openNotes.begin(); it < openNotes.end(); ++it)
     {
        // remove NULL pointers, if the Note widget is destroyed, its pointer is automatically set to null
        if(!(*it))
        {
            it = openNotes.erase(it); // skip iterator position
            continue;
        }

        if((*it) && (*it)->noteDescriptor()->uuid() == uuid)
        {
            return (*it);
        }
     }
     return 0;
}

void MainWindow::newFolder(){
     QString path = folderModel->rootPath() + "/" + tr("new folder");
     int counter = 0;
     while(QDir(path).exists())
     {
         ++counter;
         path = folderModel->rootPath() + "/" + tr("new folder (%1)").arg(QString::number(counter));
     }
     QModelIndex idx = folderModel->mkdir(folderView->rootIndex(),QDir(path).dirName());

     if(idx.isValid())
     {
         folderView->setCurrentIndex(idx);
         folderView->edit(idx); // 'open' for rename
         folderView->scrollTo(idx, QAbstractItemView::EnsureVisible);
     }
}

void MainWindow::newNote(){
     QString filePath = noteModel->rootPath() + "/" + tr("new note");
     int counter = 0;
     while(QFile::exists(filePath))
     {
         ++counter;
         filePath = noteModel->rootPath() + "/" + tr("new note (%1)").arg(QString::number(counter));
     }

     QFile file(filePath);
     if(!file.open(QIODevice::WriteOnly))
       return;
     file.close();

     QModelIndex idx = noteModel->index(filePath);
     if(idx.isValid())
     {
         noteView->setCurrentIndex(idx);
       noteView->edit(idx); // 'open' for rename
        noteView->scrollTo(idx, QAbstractItemView::EnsureVisible);
     }
}

void MainWindow::renameFolder(){
     folderView->edit(folderView->currentIndex());
}

void MainWindow::renameNote(){
     noteView->edit(noteView->currentIndex());
}

void MainWindow::removeFolder(){

     QStringList dirList = QDir(QSettings().value("noteDirPath").toString()).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

     // only needed if journal folder resides in the same folder
     //dirList.removeOne(QDir(QSettings().value("journalFolderPath").toString()).dirName());

     // keep at least one folder
     if(dirList.size() == 1)
     {
        QMessageBox::information(this,tr("One notebook must remain"),tr("At least one notebook must remain."));
        return;
     }

     QModelIndex idx = folderView->currentIndex();

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

     //folderList->setRowHidden(idx.row(),true);
     QModelIndex idxAt = folderView->indexAt(QPoint(0,0));
     if(!idxAt.isValid())
     return;

     folderView->selectionModel()->select(idxAt,QItemSelectionModel::Select);
     setCurrentFolder(idxAt);
#endif

//TODO: check why:
//QInotifyFileSystemWatcherEngine::addPaths: inotify_add_watch failed: Datei oder Verzeichnis nicht gefunden
//QFileSystemWatcher: failed to add paths: /home/hakaishi/.nobleNote/new folder
}

void MainWindow::removeNote(){
     if(QMessageBox::warning(this,tr("Delete note"),
         tr("Do you really want to delete the note \"%1\"?").arg(noteModel->fileName(noteView->currentIndex())),
           QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
       return;
     noteModel->remove(noteView->currentIndex());
}

void MainWindow::importXmlNotes()
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

void MainWindow::showContextMenuFolder(const QPoint &pos){
     QPoint globalPos = folderView->mapToGlobal(pos);

     QMenu menu;

     if(!folderView->indexAt(pos).isValid()) // if index doesn't exists at position
     {
         QAction* addNewF = new QAction(tr("&New folder"), &menu);
         connect(addNewF, SIGNAL(triggered()), this, SLOT(newFolder()));
         menu.addAction(addNewF);
     }
     if(folderView->indexAt(pos).isValid()) // if index exists at position
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

void MainWindow::showContextMenuNote(const QPoint &pos){
     QPoint globalPos = noteView->mapToGlobal(pos);

     QMenu menu;

     if(!noteView->indexAt(pos).isValid() &&
        !(noteModel->sourceModel() == findNoteModel)) // if index doesn't exists at position
     {
         QAction* addNewN = new QAction(tr("&New note"), &menu);
         connect(addNewN, SIGNAL(triggered()), this, SLOT(newNote()));
         menu.addAction(addNewN);
     }
     if(noteView->indexAt(pos).isValid()) // if index exists at position
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

void MainWindow::keyPressEvent(QKeyEvent *k){
     if(k->key() == Qt::Key_Delete){
       if(noteView->hasFocus())
         removeNote();
       if(folderView->hasFocus())
         removeFolder();
     }
}

void MainWindow::about()
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
