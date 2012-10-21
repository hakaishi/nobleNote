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
#include "listview.h"
#include "welcome.h"
#include "note.h"
#include "findfilemodel.h"
#include "filesystemmodel.h"
#include "preferences.h"
#include "lineedit.h"
#include "findfilesystemmodel.h"
#include "highlighter.h"
#include "notedescriptor.h"
#include "htmlnotereader.h"
#include "fileiconprovider.h"
#include "textsearchtoolbar.h"
#include "backup.h"
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
#include <QPushButton>
#include <QtConcurrentMap>
#include "flickcharm.h"

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
     toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
     toolBar->addAction(actionNew_folder);
     toolBar->addAction(actionRename_folder);
     toolBar->addAction(actionDelete_folder);
     toolBar->addSeparator();
     toolBar->addAction(actionNew_note);
     toolBar->addAction(actionRename_note);
     toolBar->addAction(actionDelete_note);
     toolBar->addSeparator();
     toolBar->addAction(actionHistory);
     toolBar->addAction(actionTrash);
     toolBar->addAction(actionConfigure);
     actionShowToolbar->setChecked(QSettings().value("mainwindow_toolbar_visible", true).toBool());
     toolBar->setVisible(QSettings().value("mainwindow_toolbar_visible", true).toBool());

   //Configuration file
     QSettings settings; // ini format does save but in the executables directory, use native format
     if(!settings.isWritable())
       QMessageBox::critical(this,tr("Settings not writable"),tr("W: nobelNote settings not writable!"));
     if(!settings.value("import_path").isValid())
       settings.setValue("import_path", QDir::homePath());
     if(!settings.value("root_path").isValid()){ // root path has not been set before
       welcome = new Welcome(this);
       welcome->exec();
       adjustAndSetBackupDirPath();
     }

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
     folderModel->setSortCaseSensitivity(Qt::CaseInsensitive);
     FileSystemModel *folderFSModel = new FileSystemModel(this);
     folderFSModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
     folderFSModel->setReadOnly(false);
     folderFSModel->setIconProvider(new FileIconProvider());
     folderModel->setSourceModel(folderFSModel);

     noteFSModel = new FileSystemModel(this);
     noteFSModel->setFilter(QDir::Files);
     noteFSModel->setReadOnly(false);

     //foreach(QString str, noteFSModel->mimeTypes())
     //    qDebug() << "mime types: " << str;

     findNoteModel = new FindFileModel(this);

     noteModel = new FindFileSystemModel(this);
     noteModel->setSortCaseSensitivity(Qt::CaseInsensitive);
     noteModel->setSourceModel(noteFSModel);

     folderView = new ListView(splitter);
     noteView = new ListView(splitter);

     // FlickCharm needs this mode
     folderView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
     noteView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

     flickCharm = new FlickCharm(this);

     QList<ListView*> listViews;
     listViews << folderView << noteView;
     foreach(ListView* list, listViews) // add drag drop options
     {
        if(QSettings().value("kinetic_scrolling", false).toBool())
        {
            flickCharm->activateOn(list);// enable kinetic scrolling for touchscreens
        }
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
     folderView->setEditTriggers(ListView::EditKeyPressed);
     noteView->setEditTriggers(ListView::EditKeyPressed);
     noteView->setModel(noteModel);
     noteView->setSelectionMode(QAbstractItemView::ExtendedSelection);

     writeStandardPaths();


     connect(folderView,SIGNAL(activated(QModelIndex)),this,SLOT(folderActivated(QModelIndex)));
     connect(folderView,SIGNAL(clicked(QModelIndex)),this,SLOT(folderActivated(QModelIndex)));
     connect(folderView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,
          QItemSelection)),this,SLOT(folderActivated(QItemSelection,QItemSelection))); //Wrapper
     connect(folderView->itemDelegate(),SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
             this,SLOT(folderRenameFinished(QWidget*,QAbstractItemDelegate::EndEditHint)));
     connect(folderView, SIGNAL(customContextMenuRequested(const QPoint &)),
             this, SLOT(showContextMenuFolder(const QPoint &)));
     connect(searchName, SIGNAL(textChanged(const QString)), this, SLOT(find()));
     connect(searchText, SIGNAL(textChanged(const QString)), this, SLOT(find()));
     connect(noteFSModel,SIGNAL(fileRenamed(QString,QString,QString)),this,SLOT(noteRenameFinished(QString,QString,QString)));
     //     connect(folderList, SIGNAL(clicked(const QModelIndex &)), this,
     //       SLOT(setCurrentFolder(const QModelIndex &)));
     //     connect(folderList,SIGNAL(activated(QModelIndex)), this,
     //       SLOT(setCurrentFolder(QModelIndex)));
     connect(noteView,SIGNAL(activated(QModelIndex)), this,
             SLOT(openNote(QModelIndex)));
     connect(noteView, SIGNAL(customContextMenuRequested(const QPoint &)),
             this, SLOT(showContextMenuNote(const QPoint &)));
     connect(noteView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,
          QItemSelection)),this,SLOT(noteActivated(QItemSelection,QItemSelection))); //Wrapper
     connect(noteView,SIGNAL(activated(QModelIndex)),this,SLOT(noteActivated(QModelIndex)));
     connect(noteView,SIGNAL(clicked(QModelIndex)),this,SLOT(noteActivated(QModelIndex)));
     connect(noteView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,
          QItemSelection)), this, SLOT(enableNoteMenu(QItemSelection,QItemSelection)));

#ifndef NO_SYSTEM_TRAY_ICON
     connect(TIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
             this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason))); //handles systray-symbol
     connect(minimizeRestoreAction, SIGNAL(triggered()), this, SLOT(tray_actions()));
#endif

     connect(actionImport,SIGNAL(triggered()),this,SLOT(importDialog()));
     connect(actionQuit, SIGNAL(triggered()), this, SLOT(quit()));

     connect(actionTrash, SIGNAL(triggered()), this, SLOT(showBackupWindow()));
     connect(quit_action, SIGNAL(triggered()), this, SLOT(quit())); //contextmenu "Quit" for the systray
     connect(actionNew_folder, SIGNAL(triggered()), this, SLOT(newFolder()));
     connect(actionRename_folder, SIGNAL(triggered()), this, SLOT(renameFolder()));
     connect(actionDelete_folder, SIGNAL(triggered()), this, SLOT(removeFolder()));
     connect(actionNew_note, SIGNAL(triggered()), this, SLOT(newNote()));
     connect(actionRename_note, SIGNAL(triggered()), this, SLOT(renameNote()));
     connect(actionDelete_note, SIGNAL(triggered()), this, SLOT(removeNote()));
     connect(action_Cut, SIGNAL(triggered()), this, SLOT(getCutFiles()));
     connect(action_Paste, SIGNAL(triggered()), this, SLOT(pasteFiles()));
     connect(actionShowToolbar, SIGNAL(toggled(bool)), toolBar, SLOT(setVisible(bool)));
     connect(toolBar, SIGNAL(visibilityChanged(bool)), actionShowToolbar, SLOT(setChecked(bool)));
     //connect(actionHistory, SIGNAL(triggered()), this, SLOT(showHistory()));

     connect(actionConfigure, SIGNAL(triggered()), this, SLOT(showPreferences()));
     connect(actionAbout,SIGNAL(triggered()),this,SLOT(about()));
}

MainWindow::~MainWindow(){}

void MainWindow::adjustAndSetBackupDirPath()
{
     QString str = QSettings().value("root_path").toString();
     str.replace(QString("/"), QString("_"));
   #ifdef Q_OS_WIN32
     str.prepend("_");
     str.remove(":");

     QString backupPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation) +
                              "/nobleNote/backups";
   #else
     QString backupPath = QDir::homePath() + "/.local/share/nobleNote/backups";
   #endif

     QSettings().setValue("backup_dir_path", backupPath + str);
}

void MainWindow::changeRootIndex(){
     if(!openNotes.isEmpty()){
        foreach(QWidget *note, openNotes)
            if(note)
                note->close();
        openNotes.clear();
     }
     adjustAndSetBackupDirPath();
     writeStandardPaths();
}

void MainWindow::writeStandardPaths(){
     if(!QDir(QSettings().value("root_path").toString()).exists())
       QDir().mkpath(QSettings().value("root_path").toString());

     if(!QDir(QSettings().value("backup_dir_path").toString()).exists())
       QDir().mkpath(QSettings().value("backup_dir_path").toString());

     folderView->setRootIndex(folderModel->setRootPath(QSettings().value("root_path").toString()));

     // make sure there's at least one folder
     QStringList dirList = QDir(QSettings().value("root_path").toString()).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
     if(dirList.isEmpty())
     {
         QString defaultDirName = tr("default");
         QDir(QSettings().value("root_path").toString()).mkdir(defaultDirName);
         noteView->setRootIndex(noteModel->setRootPath(QSettings().value("root_path").toString() + "/" + defaultDirName)); // set default dir as current note folder
     }
     else
         noteView->setRootIndex(noteModel->setRootPath(QSettings().value("root_path").toString() + "/" + dirList.first())); // dirs exist, set first dir as current note folder

     //Select the first folder
     if(!dirList.isEmpty())
     {
          folderView->selectionModel()->select(folderModel->index(QSettings().value(
               "root_path").toString() + "/" + dirList.first()),QItemSelectionModel::Select);
     }
}

void MainWindow::enableNoteMenu(const QItemSelection &selected, const QItemSelection &deselected)
{
     Q_UNUSED(deselected);
     actionRename_note->setDisabled(selected.isEmpty());
     actionDelete_note->setDisabled(selected.isEmpty());
}

void MainWindow::showPreferences()
{
     if(!pref)
     {
          pref = new Preferences(this);
          connect(pref, SIGNAL(pathChanged()), this, SLOT(changeRootIndex()));
          connect(pref,SIGNAL(kineticScrollingEnabledChanged(bool)),this,SLOT(setKineticScrollingEnabled(bool)));
     }
     pref->show();
}

void MainWindow::showBackupWindow()
{
     if(!backup)
       backup = new Backup(this);
     backup->show();
}

void MainWindow::find()
{
    // disable note toolbar buttons because the current notes are not longer visible with the findNoteModel

         noteModel->setSourceModel(findNoteModel);
         noteModel->clear(); // if findNoteModel already set, clear old found list
         noteModel->findInFiles(searchName->text(),searchText->text(),folderModel->rootPath());

         actionNew_note->setDisabled(true);
}

void MainWindow::folderRenameFinished(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
     if(folderView->selectionModel()->selectedIndexes().isEmpty())
       return;
     Q_UNUSED(editor);
     if(hint != QAbstractItemDelegate::RevertModelCache) // canceled editing
     {
         QString currFolderPath = folderModel->filePath(folderView->selectionModel()->selectedIndexes().first());
         folderModel->sort(0);
         folderView->setCurrentIndex(folderModel->index(currFolderPath));

         // set current folder
         noteModel->setSourceModel(noteFSModel);
         noteView->setRootIndex(noteModel->setRootPath(currFolderPath));
     }

     folderView->scrollTo(folderView->selectionModel()->selectedIndexes().first());
}

void MainWindow::noteRenameFinished(const QString & path, const QString & oldName, const QString & newName)
{
     QString filePath = noteModel->filePath(noteView->currentIndex());
     Note * w = noteWindow(path + "/" + oldName);
     if(w)
        w->setWindowTitle(QFileInfo(path + "/" + newName).baseName());

     noteView->model()->sort(0);
     noteView->setCurrentIndex(noteModel->index(filePath));

     noteView->scrollTo(noteView->selectionModel()->selectedIndexes().first());
}

void MainWindow::folderActivated(const QModelIndex &selected)
{
     // clear search line edits
     searchName->clear();
     searchText->clear();

     actionNew_note->setEnabled(true);
     actionRename_note->setDisabled(true);
     actionDelete_note->setDisabled(true);

     noteModel->setSourceModel(noteFSModel);
     noteView->setRootIndex(noteModel->setRootPath(folderModel->filePath(selected)));
}

void MainWindow::folderActivated(const QItemSelection &selected, const QItemSelection &deselected) //Wrapper
{
     Q_UNUSED(deselected);
     if(selected.indexes().isEmpty())
       return;
     folderActivated(selected.indexes().first()); //we only need one - anyone is fine
}

void MainWindow::noteActivated(const QModelIndex &selected)
{
     Q_UNUSED(selected);
     actionRename_note->setEnabled(true);
     actionDelete_note->setEnabled(true);
}

void MainWindow::noteActivated(const QItemSelection &selected, const QItemSelection &deselected)
{
     Q_UNUSED(deselected);
     if(selected.indexes().isEmpty())
       return;
     noteActivated(selected.indexes().first()); //we only need one - anyone is fine
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
     if(QSettings().contains("splitter"))
       splitter->restoreState(QSettings().value("splitter").toByteArray());
     if(QSettings().value("open_notes").isValid())
       foreach(QString path, QSettings().value("open_notes").toStringList())
       {
          Note* note = new Note(path);
          openNotes += note;
          note->setObjectName(path);
          note->show();
       }
     QMainWindow::showEvent(show_window);
}

void MainWindow::hideEvent(QHideEvent* window_hide){
     minimizeRestoreAction->setText(tr("&Restore"));
     QMainWindow::hideEvent(window_hide);
}

void MainWindow::closeEvent(QCloseEvent* window_close){
     if(QSettings().value("dont_quit_on_close").toBool())
     {
       hide();
     }
     else{
       QSettings().setValue("mainwindow_size", saveGeometry());
       QSettings().setValue("splitter", splitter->saveState());
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
     Q_UNUSED(index);
     openAllNotes();
}

void MainWindow::openAllNotes(){
     QList<QModelIndex> indexes = noteView->selectionModel()->selectedIndexes();
     foreach(QModelIndex ind, indexes)
     {
          if(!ind.isValid()) // default constructed model index
            ind = noteView->currentIndex();

          QString notePath = noteModel->filePath(ind);
          if(!QFileInfo(notePath).exists())
          {
              QMessageBox::warning(this,tr("Note does not exist"), tr("The selected note cannot be opened because it has been moved or renamed"));
              return;
          }

           // check if the notePath is already used in a open note
          QWidget* w = noteWindow(notePath);
          if(w)
          {
              w->activateWindow();  // highlight the note window
              return;
          }

          Note* note = new Note(notePath);
          openNotes += note;
          note->setObjectName(notePath);
          QStringList savedOpenNoteList;
          if(QSettings().value("open_notes").isValid())
            savedOpenNoteList = QSettings().value("open_notes").toStringList();
          savedOpenNoteList.append(notePath);
          QSettings().setValue("open_notes",savedOpenNoteList);
          if(QSettings().value("kinetic_scrolling", false).toBool())
          {
              flickCharm->activateOn(note->textEdit());
          }
          if(noteModel->sourceModel() == findNoteModel){
            note->highlightText(searchText->text());
            note->searchbarVisible = true;
            note->setSearchBarText(searchText->text());
          }
          note->show();
     }
}

void MainWindow::openNoteSource()
{
    QModelIndex ind = noteView->currentIndex();

    QFile file(noteModel->filePath(ind));
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
           return;

    QMainWindow * mainWindow = new QMainWindow();
    mainWindow->setAttribute(Qt::WA_DeleteOnClose);
    QTextEdit * textEdit = new QTextEdit(mainWindow);
    textEdit->setReadOnly(true);
    textEdit->setPlainText(QTextStream(&file).readAll());

    if(QSettings().value("kinetic_scrolling", false).toBool())
    {
        flickCharm->activateOn(textEdit);
    }

    mainWindow->setCentralWidget(textEdit);
    TextSearchToolbar * searchBar = new TextSearchToolbar(textEdit,mainWindow);
    mainWindow->addToolBar(searchBar);
    mainWindow->resize(QSettings().value("note_editor_default_size",QSize(335,250)).toSize());
//    searchBar->searchLine()->setFocus();
//    searchBar->setFocusPolicy(Qt::TabFocus);

    openNotes += mainWindow;
    mainWindow->show();
}

Note *MainWindow::noteWindow(const QString &filePath)
{
     QUuid uuid = HtmlNoteReader::uuid(filePath);
     for(QList<QPointer<QWidget> >::Iterator it = openNotes.begin(); it < openNotes.end(); ++it)
     {
        // remove NULL pointers, if the Note widget is destroyed, its pointer is automatically set to null
        if(!(*it))
        {
            it = openNotes.erase(it); // skip iterator position
            continue;
        }

        Note * note = qobject_cast<Note*>(*it);
        if(note && note->noteDescriptor()->uuid() == uuid)
        {
            return note;
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
     }
     folderModel->sort(0);
     folderView->scrollTo(folderView->selectionModel()->selectedIndexes().first());
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
     }
     noteView->model()->sort(0);
     noteView->scrollTo(noteView->selectionModel()->selectedIndexes().first());
}

void MainWindow::renameFolder(){
     if(!folderView->selectionModel()->selectedIndexes().isEmpty())
       folderView->edit(folderView->selectionModel()->selectedIndexes().first());
}

void MainWindow::renameNote(){
     if(!folderView->selectionModel()->selectedIndexes().isEmpty())
       noteView->edit(noteView->selectionModel()->selectedIndexes().first());
}

void MainWindow::removeFolder(){
     if(folderView->selectionModel()->selectedIndexes().isEmpty())
       return;

     QStringList dirList = QDir(QSettings().value("root_path").toString()).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

     // only needed if journal folder resides in the same folder
     //dirList.removeOne(QDir(QSettings().value("journalFolderPath").toString()).dirName());

     // keep at least one folder
     if(dirList.size() == 1)
     {
        QMessageBox::information(this,tr("One notebook must remain"),tr("At least one notebook must remain."));
        return;
     }

     QModelIndex idx = folderView->selectionModel()->selectedIndexes().first();

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
                qWarning(qPrintable(QString("Could not delete ") + name));
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
     noteView->setRootIndex(noteModel->setRootPath(folderModel->filePath(idxAt)));
#endif

//TODO: check why:
//QInotifyFileSystemWatcherEngine::addPaths: inotify_add_watch failed: Datei oder Verzeichnis nicht gefunden
//QFileSystemWatcher: failed to add paths: /home/hakaishi/.nobleNote/new folder
}

void MainWindow::removeNote(){
     if(noteView->selectionModel()->selectedIndexes().isEmpty())
       return;
     QString names;
     foreach(QString name, noteModel->fileNames(noteView->selectionModel()->selectedIndexes()))
          names += "\"" + name + "\"\n";
     if(QMessageBox::warning(this,tr("Delete note"),
         tr("Do you really want to delete the following note(s)?\n%1").arg(names),
           QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
       return;
     noteModel->removeList(noteView->selectionModel()->selectedIndexes());
}

void MainWindow::setKineticScrollingEnabled(bool b)
{
    QList<QAbstractScrollArea* > widgets;
    widgets << noteView << folderView;
    foreach(QWidget * w, openNotes)
        if(Note * note = qobject_cast<Note*>(w))
            widgets << note->textEdit();
    if(b)
    {
        foreach(QAbstractScrollArea* widget, widgets)
            if(widget)
                flickCharm->activateOn(widget);
    }
    else
    {
        foreach(QAbstractScrollArea* widget, widgets)
            if(widget)
                flickCharm->deactivateFrom(widget);
    }
}

void MainWindow::importDialog()
{
     if(fileDialog)
       return;

     importFiles.clear(); //remove old files

     fileDialog = new QFileDialog(this, tr("Select one or more files to open"),
                      QSettings().value("import_path").toString(), tr("Notes")+"(*.note)");
     fileDialog->setViewMode(QFileDialog::Detail);
     fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
     fileDialog->setFileMode(QFileDialog::ExistingFiles);
     fileDialog->setOption(QFileDialog::ReadOnly);
     fileDialog->show();
     QObject::connect(fileDialog, SIGNAL(accepted()), this, SLOT(importXmlNotes()));
     QObject::connect(fileDialog, SIGNAL(rejected()), fileDialog, SLOT(deleteLater()));
}

void MainWindow::importXmlNotes()
{
     importFiles = fileDialog->selectedFiles();
     if(importFiles.isEmpty())
        return;

     QSettings().setValue("import_path",QFileInfo(importFiles.last()).absolutePath());

     dialog = new QProgressDialog(this);
     dialog->setLabelText(QString(tr("Importing notes...")));

     progressReceiver = new ProgressReceiver(this);
     noteImporter.path = QSettings().value("root_path").toString();
     noteImporter.p = progressReceiver;

     futureWatcher = new QFutureWatcher<void>(this);
     futureWatcher->setFuture(QtConcurrent::map(importFiles, noteImporter));

     QObject::connect(progressReceiver,SIGNAL(valueChanged(int)),dialog, SLOT(setValue(int)));
     QObject::connect(futureWatcher, SIGNAL(finished()), dialog, SLOT(reset()));
     QObject::connect(dialog, SIGNAL(canceled()), futureWatcher, SLOT(cancel()));
     QObject::connect(futureWatcher, SIGNAL(canceled()), futureWatcher, SLOT(deleteLater()));
     QObject::connect(futureWatcher, SIGNAL(finished()), futureWatcher, SLOT(deleteLater()));
     QObject::connect(futureWatcher, SIGNAL(canceled()), dialog, SLOT(deleteLater()));
     QObject::connect(futureWatcher, SIGNAL(finished()), dialog, SLOT(deleteLater()));
     QObject::connect(futureWatcher, SIGNAL(canceled()), progressReceiver, SLOT(deleteLater()));
     QObject::connect(futureWatcher, SIGNAL(finished()), progressReceiver, SLOT(deleteLater()));
     QObject::connect(futureWatcher, SIGNAL(canceled()), fileDialog, SLOT(deleteLater()));
     QObject::connect(futureWatcher, SIGNAL(finished()), fileDialog, SLOT(deleteLater()));

     dialog->show();
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
         menu.addSeparator();
         menu.addAction(action_Paste);
     }
     if(noteView->indexAt(pos).isValid()) // if index exists at position
     {
         QAction* openAll = new QAction(tr("&Open notes"), &menu);
         QAction* renameN = new QAction(tr("&Rename note"), &menu);
         QAction* removeNote = new QAction(tr("&Delete note"), &menu);

         if(noteView->selectionModel()->selectedIndexes().count() == 1)
           removeNote->setText(tr("&Delete note"));
         else
           removeNote->setText(tr("&Delete notes"));
         connect(openAll, SIGNAL(triggered()), this, SLOT(openAllNotes()));
         connect(renameN, SIGNAL(triggered()), this, SLOT(renameNote()));
         connect(removeNote, SIGNAL(triggered()), this, SLOT(removeNote()));
         if(noteView->selectionModel()->selectedIndexes().count() == 1)
           menu.addAction(renameN);
         else
           menu.addAction(openAll);
         menu.addAction(removeNote);

         if(QSettings().value("show_source").toBool()) // developer option setting
         {
             QAction* showSourceAction = new QAction(tr("Show &Source"), &menu);
             connect(showSourceAction,SIGNAL(triggered()),this,SLOT(openNoteSource()));
             if(noteView->selectionModel()->selectedIndexes().count() == 1)
               menu.addAction(showSourceAction);
         }
         menu.addSeparator();
         menu.addAction(action_Cut);
     }
     menu.exec(globalPos);
}

void MainWindow::getCutFiles()
{
     shortcutNoteList.clear();
     if(noteView->hasFocus())
       foreach(QModelIndex idx, noteView->selectionModel()->selectedIndexes())
          shortcutNoteList << noteModel->filePath(idx);

     if(!shortcutNoteList.isEmpty())
       action_Paste->setEnabled(true);
}

void MainWindow::pasteFiles()
{
     if(shortcutNoteList.isEmpty())
       return;

     QString copyErrorFiles;
     foreach(QString note, shortcutNoteList)
     {
          if(!QFile(note).copy(folderModel->filePath(
             folderView->selectionModel()->selectedIndexes().first())
             + "/" + QFileInfo(note).fileName()))
                     copyErrorFiles += "\"" + note + "\"\n";
          else QFile(note).remove();
     }
     if(!copyErrorFiles.isEmpty())
       QMessageBox::critical(this, tr("Copy error"), tr("Files of the same names "
                             "already exist in this folder:\n\n") + copyErrorFiles);
     shortcutNoteList.clear();
     action_Paste->setDisabled(true);
}

void MainWindow::keyPressEvent(QKeyEvent *k){
     if(k->key() == Qt::Key_Delete)
     {
       if(noteView->hasFocus())
         removeNote();
       if(folderView->hasFocus())
         removeFolder();
     }

     if(k->matches(QKeySequence::Cut))
       getCutFiles();
     if(k->matches(QKeySequence::Paste))
       pasteFiles();
     if(k->key() == Qt::Key_Escape)
       shortcutNoteList.clear();
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
