/* nobleNote, a note taking application
 * Copyright (C) 2020 Christian Metscher <hakaishi@web.de>,
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
#include "noteimporter.h"
#include "systemtraycreator.h"
#include <QTextStream>
#include <QFile>
#include <QModelIndex>
#include <QInputDialog>
#include <QMouseEvent>
#include <QMessageBox>
#include <QFileIconProvider>
#include <QList>
#include <QPushButton>
#include <QtConcurrent/QtConcurrentMap>
#include <QScroller>
#include <QStandardPaths>

MainWindow::MainWindow()
{
     setupUi(this);

     fileWatcher = new QFileSystemWatcher(this);

     createAndUpdateRecent();

   //TrayIcon
     QIcon icon = QIcon(":nobleNote");

     minimizeRestoreAction = new QAction(tr("&Restore"),this);
     actionQuitSystrayMenu = new QAction(tr("&Quit"),this);

#ifndef NO_SYSTEM_TRAY_ICON
     TIcon = new QSystemTrayIcon(this);
     TIcon->setIcon(icon);
     TIcon->show();

   //TrayIconContextMenu
     iMenu = new QMenu(this);
     
   // context menu needs at least one action else right click will not work
     iMenu->addAction(minimizeRestoreAction);
     iMenu->addAction(actionQuitSystrayMenu);
     
     TIcon->setContextMenu(iMenu);  //setting contextmenu for the systray
     
     iconCreator = new SystemTrayCreator(this);

     if(QSettings().value("open_notes_from_tray", false).toBool()){
         connect(iconCreator,&SystemTrayCreator::noteClicked,this,&MainWindow::openOneNote);
         connect(iMenu,&QMenu::aboutToShow,this,[=](){
             iconCreator->populateMenu(iMenu);
             iMenu->addSeparator();
             iMenu->addAction(minimizeRestoreAction);
             iMenu->addAction(actionQuitSystrayMenu);
         });     
     }

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

     writeBackupDirPath();

   //Search line edits
//     searchName = new LineEdit(this);
//     searchName->setPlaceholderText(tr("Search for note"));
//     gridLayout->addWidget(searchName, 1, 0);

     searchText = new LineEdit(this);
     searchText->setPlaceholderText(tr("Type to search for notes"));
     gridLayout->addWidget(searchText, 2, 0);

     splitter = new QSplitter(centralwidget);
     gridLayout->addWidget(splitter, 3, 0);

   //IconProvider
     folderIconProvider = new FileIconProvider();
     noteIconProvider = new FileIconProvider();

     folderModel = new FindFileSystemModel(this);
     folderModel->setSortCaseSensitivity(Qt::CaseInsensitive);
     FileSystemModel *folderFSModel = new FileSystemModel(this);
     folderFSModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
     folderFSModel->setReadOnly(false);
     folderFSModel->setIconProvider(folderIconProvider);
     folderModel->setSourceModel(folderFSModel);

     noteFSModel = new FileSystemModel(this);
     noteFSModel->setFilter(QDir::Files);
     noteFSModel->setReadOnly(false);
     noteFSModel->setIconProvider(noteIconProvider);

     findNoteModel = new FindFileModel(this);

     noteModel = new FindFileSystemModel(this);
     noteModel->setSortCaseSensitivity(Qt::CaseInsensitive);
     noteModel->setSourceModel(noteFSModel);

     folderView = new QListView(splitter);
     noteView = new QListView(splitter);

     // FlickCharm needs this mode
     folderView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
     noteView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

     const QList<QListView*> listViews = QList<QListView*>() << folderView << noteView;

     for(QListView* list : listViews) // add drag drop options
     {
        if(QSettings().value("kinetic_scrolling", false).toBool())
        {
            QScroller::grabGesture(list->viewport(), QScroller::LeftMouseButtonGesture);
        }
        list->setContextMenuPolicy(Qt::CustomContextMenu);
        //list->setSelectionMode(QAbstractItemView::SingleSelection); // single item can be draged or droped
        list->setDragDropMode(QAbstractItemView::DragDrop);
        list->viewport()->setAcceptDrops(true);
        list->setDropIndicatorShown(true);
        list->setDefaultDropAction(Qt::CopyAction);
        list->setSelectionBehavior(QAbstractItemView::SelectRows);
     }
     noteView->setDragEnabled(true);
     folderView->setDragEnabled(false);

     folderView->setModel(folderModel);
     folderView->setEditTriggers(QListView::EditKeyPressed);
     noteView->setEditTriggers(QListView::EditKeyPressed);
     noteView->setModel(noteModel);
     noteView->setSelectionMode(QAbstractItemView::ExtendedSelection);

     noteImporter = new NoteImporter(this);

     makeStandardPaths(); // mkpath the standard paths if they do not exist already

     actionRename_note->setIconVisibleInMenu(false);

     connect(folderView,SIGNAL(activated(QModelIndex)),this,SLOT(folderActivated(QModelIndex)));
     connect(folderView,SIGNAL(clicked(QModelIndex)),this,SLOT(folderActivated(QModelIndex)));
     connect(folderView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,
          QItemSelection)),this,SLOT(folderActivated(QItemSelection,QItemSelection))); //Wrapper
     connect(folderView->itemDelegate(),SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
             this,SLOT(folderRenameFinished(QWidget*,QAbstractItemDelegate::EndEditHint)));
     connect(folderView, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(showContextMenuFolder(QPoint)));
     //connect(searchName, SIGNAL(textChanged(const QString)), this, SLOT(find()));
     connect(searchText, SIGNAL(textChanged(QString)), this, SLOT(find()));
     connect(noteFSModel,SIGNAL(fileRenamed(QString,QString,QString)),this,SLOT(noteRenameFinished(QString,QString,QString)));
     connect(fileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(createAndUpdateRecent()));
     //     connect(folderList, SIGNAL(clicked(const QModelIndex &)), this,
     //       SLOT(setCurrentFolder(const QModelIndex &)));
     //     connect(folderList,SIGNAL(activated(QModelIndex)), this,
     //       SLOT(setCurrentFolder(QModelIndex)));
     connect(noteView,SIGNAL(activated(QModelIndex)), this,
             SLOT(openNote(QModelIndex)));
     connect(noteView, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(showContextMenuNote(QPoint)));
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

     connect(actionImport,SIGNAL(triggered()),noteImporter,SLOT(importDialog()));
     connect(actionQuitFileMenu, SIGNAL(triggered()), this, SLOT(quit()));

     connect(actionTrash, SIGNAL(triggered()), this, SLOT(showBackupWindow()));
     connect(actionQuitSystrayMenu, SIGNAL(triggered()), this, SLOT(quit()));
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
     connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));

     connect(searchText, SIGNAL(sendCleared()), this, SLOT(selectFolder()));

     connect(folderFSModel, SIGNAL(droppedAFile()), this, SLOT(createAndUpdateRecent())); //moving a note with
                                                                 //drag and drop removes it from the recent files

     connect(folderFSModel,&FileSystemModel::droppedAFile, this, [this](){
         QString text = searchText->text();
         if(!text.isEmpty())
         {
             find(false);
         }
     });
}

MainWindow::~MainWindow()
{
    delete folderIconProvider;
    delete noteIconProvider;
}

void MainWindow::selectFolder()
{
     if(folderView->selectionModel()->selectedIndexes().isEmpty())
       return;
     folderActivated(folderView->selectionModel()->selectedIndexes().first());
}

void MainWindow::writeBackupDirPath() //generates a backup path according to OS and portability
{
     QString backupPath;
     QString suffix = "";

     if(!QSettings().value("isPortable",false).toBool()) //if not portable
     {
       suffix = QSettings().value("root_path").toString();

      #ifdef Q_OS_WIN32
       suffix.prepend(QDir::separator()); //we want to seperate "backup" from the rest of the path (see below)
       suffix.remove(":"); //removing illegal character from path
      #endif

       suffix.replace(QDir::separator(), "_");

       backupPath = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation).first();

     // reduce extraordinary long path, replaces .../nobleNote/nobleNote/... with .../nobleNote/...

        QString doubleNobleNote =
                (QDir::separator() + qApp->organizationName() + QDir::separator() + qApp->applicationName());
        QString singleNobleNote = QDir::separator() +qApp->applicationName();

        // use unified separators, because QDir::separator() is platform native (\ on Windows)
        // but QStandardPaths uses / on Windows and Linux
        backupPath = QDir().toNativeSeparators(backupPath);
        doubleNobleNote = QDir().toNativeSeparators(doubleNobleNote);
        singleNobleNote = QDir().toNativeSeparators(singleNobleNote);

       if(backupPath.contains(doubleNobleNote))
         backupPath.replace(doubleNobleNote,singleNobleNote);
     }
     else //if portable
       backupPath = qApp->applicationDirPath();

     QSettings().setValue("backup_dir_path", backupPath + QDir::separator() + "backups" + suffix);
     //note that suffix will be "" if portable
}

void MainWindow::changeRootIndex(){
     if(!openNotes.isEmpty()){
        for(QWidget *note : openNotes)
            if(note)
            {
                note->close();
            }

        openNotes.clear();
     }
     writeBackupDirPath();
     makeStandardPaths();
}

void MainWindow::makeStandardPaths(){
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
         noteView->setRootIndex(noteModel->setRootPath(QSettings().value("root_path").toString() + QDir::separator() + defaultDirName)); // set default dir as current note folder
     }
     else
         noteView->setRootIndex(noteModel->setRootPath(QSettings().value("root_path").toString() + QDir::separator() + dirList.first())); // dirs exist, set first dir as current note folder

     //Select the first folder
     if(!dirList.isEmpty())
     {
          folderView->selectionModel()->select(folderModel->index(QSettings().value(
               "root_path").toString() + QDir::separator() + dirList.first()),QItemSelectionModel::Select);
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
          connect(pref, SIGNAL(recentCountChanged()), this, SLOT(createAndUpdateRecent()));
          connect(pref, SIGNAL(accepted()), this, SLOT(settingsAccepted()));
     }
     pref->show();
}

void MainWindow::settingsAccepted(){
#ifndef NO_SYSTEM_TRAY_ICON
    iconCreator->disconnect();
    if(QSettings().value("open_notes_from_tray", false).toBool()){
         connect(iconCreator,&SystemTrayCreator::noteClicked,this,&MainWindow::openOneNote);
         iMenu->disconnect(SIGNAL(aboutToShow()));
         connect(iMenu,&QMenu::aboutToShow,this,[=](){
             iconCreator->populateMenu(iMenu);
             iMenu->addSeparator();
             iMenu->addAction(minimizeRestoreAction);
             iMenu->addAction(actionQuitSystrayMenu);
         });    
     }
     else{
        iMenu->clear();
        iMenu->disconnect(SIGNAL(aboutToShow()));
        iMenu->addAction(minimizeRestoreAction);
        iMenu->addAction(actionQuitSystrayMenu);
     }
#endif
}

void MainWindow::showBackupWindow()
{
     if(!backup)
       backup = new Backup(this);
}

void MainWindow::find(bool waitCursorEnabled)
{
    // disable note toolbar buttons because the current notes are not longer visible with the findNoteModel

         noteView->viewport()->setAcceptDrops(false);

         noteModel->setSourceModel(findNoteModel);
         noteModel->clear(); // if findNoteModel already set, clear old found list
         //noteModel->findInFiles(searchName->text(),searchText->text(),folderModel->rootPath());
         noteModel->findInFiles(searchText->text(),searchText->text(),folderModel->rootPath(),waitCursorEnabled);

         actionNew_note->setDisabled(true);
}

void MainWindow::folderRenameFinished(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
     QString newNotebookName = folderView->selectionModel()->selectedRows().first().data(Qt::DisplayRole).toString();
     QStringList recent = QSettings().value("Recent_notes").toStringList();

     for(int i = 0; i < recent.size(); i++)
     {
          QString fileName = QFileInfo(recent[i]).fileName();
          QString strippedPath = recent[i];
          strippedPath.chop(fileName.size() + getToBerenamedNotebook.size() +1);
          if(!strippedPath.contains(strippedPath + getToBerenamedNotebook))
            recent[i] = strippedPath + newNotebookName + QDir::separator() + fileName;
     }
     QSettings().setValue("Recent_notes", recent);

     if(folderView->selectionModel()->selectedRows().isEmpty())
       return;
     Q_UNUSED(editor);
     if(hint != QAbstractItemDelegate::RevertModelCache) // canceled editing
     {
         QString currFolderPath = folderModel->filePath(folderView->selectionModel()->selectedRows().first());
         folderModel->sort(0);
         folderView->setCurrentIndex(folderModel->index(currFolderPath));

         // set current folder
         noteModel->setSourceModel(noteFSModel);
         noteView->setRootIndex(noteModel->setRootPath(currFolderPath));
     }

     folderView->scrollTo(folderView->selectionModel()->selectedRows().first());
}

void MainWindow::noteRenameFinished(const QString & path, const QString & oldName, const QString & newName)
{
     QString filePath = noteModel->filePath(noteView->currentIndex());
     Note * w = noteWindow(path + QDir::separator() + oldName);
     if(w)
        w->setWindowTitle(QFileInfo(path + QDir::separator() + newName).baseName());

     noteView->model()->sort(0);
     noteView->setCurrentIndex(noteModel->index(filePath));

     noteView->scrollTo(noteView->selectionModel()->selectedRows().first());

     QStringList recent = QSettings().value("Recent_notes").toStringList();

     for(int i = 0; i < recent.size(); i++)
          if(recent[i].contains(path + QDir::separator() + oldName))
            recent.replace(i, path + QDir::separator() + newName);
     QSettings().setValue("Recent_notes", recent);
}

void MainWindow::folderActivated(const QModelIndex &selected)
{
     // clear search line edits
     //searchName->clear();
     searchText->clear();

     actionNew_note->setEnabled(true);
     actionRename_note->setDisabled(true);
     actionDelete_note->setDisabled(true);

     noteModel->setSourceModel(noteFSModel);
     noteView->setRootIndex(noteModel->setRootPath(folderModel->filePath(selected)));

     noteView->viewport()->setAcceptDrops(true);
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
void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
     if(reason == QSystemTrayIcon::Trigger)
       tray_actions();
}
#endif

#ifndef NO_SYSTEM_TRAY_ICON
void MainWindow::tray_actions()
{
     if(isMinimized() || isHidden())  //in case that the window is minimized or hidden
       showNormal();
     else
       hide();
}
#endif

/*#ifndef NO_SYSTEM_TRAY_ICON
void MainWindow::updateMyTrayIcon()
{
     trayMenu
}
#endif*/

void MainWindow::showEvent(QShowEvent* show_window)
{
     minimizeRestoreAction->setText(tr("&Minimize"));
     if(QSettings().contains("mainwindow_size"))
       restoreGeometry(QSettings().value("mainwindow_size").toByteArray());
     if(QSettings().contains("splitter"))
       splitter->restoreState(QSettings().value("splitter").toByteArray());

     showOpenNotes();

     QMainWindow::showEvent(show_window);
}

void MainWindow::showOpenNotes() //do not confuse with "recent"
{
     if(QSettings().value("open_notes").isValid())
     {
       const QStringList list = QSettings().value("open_notes").toStringList();
       for(QString path : list)
       {
           // check if the notePath is already used in a open note
          if(!noteIsOpen(path) && QFile(path).exists())
          {
               Note* note = new Note(path);
               openNotes += note;
               note->setObjectName(path);
               note->showAfterLoaded();
          }
       }
     }
}

void MainWindow::hideEvent(QHideEvent* window_hide)
{
     minimizeRestoreAction->setText(tr("&Restore"));
     QMainWindow::hideEvent(window_hide);
}

void MainWindow::closeEvent(QCloseEvent* window_close)
{
     if(QSettings().value("dont_quit_on_close").toBool())
     {
       hide();
     }
     else{
       QSettings().setValue("mainwindow_size", saveGeometry());
       QSettings().setValue("splitter", splitter->saveState());

       // find widgets that still are saving their settings and let their futuures (worker threads) finish
       QListIterator<QPointer<QWidget> > it(openNotes);
       while(it.hasNext())
       {
           QPointer<QWidget> ptr = it.next();
           QWidget * widget = ptr.data();
           Note * note = qobject_cast<Note*>(widget);
           if(note != NULL)
           {
               // calls close event (this is normally not called, because Notes are no childs of this)
               // TODO check why Notes are no children of this

               note->close();

               // close() calls the closeEvent, this method assumes the user has closed the window
               // so we manually have to re-add this note to the list of open notes.
               Note::addToOpenNoteList(note->noteDescriptor()->filePath());
           }
       }
       qApp->setQuitOnLastWindowClosed(true);
     }

     QMainWindow::closeEvent(window_close);
}

void MainWindow::quit()
{
     QSettings().setValue("mainwindow_size", saveGeometry());
     QSettings().setValue("mainwindow_toolbar_visible", actionShowToolbar->isChecked());
     qApp->setQuitOnLastWindowClosed(true);
     qApp->closeAllWindows();
     close();
}

bool MainWindow::noteIsOpen(const QString &path)
{
     bool isOpen = false;
     QWidget* w = noteWindow(path);
     if(w)
     {
          isOpen = true;
          w->activateWindow();  // highlight the note window
     }
     return isOpen;
}

void MainWindow::openNote(const QModelIndex &index /* = new QModelIndex*/){
     Q_UNUSED(index);
     openAllNotes();
}

void MainWindow::openAllNotes(){
     const QList<QModelIndex> indexes = noteView->selectionModel()->selectedRows();
     for(QModelIndex ind :  indexes)
     {
          if(!ind.isValid()) // default constructed model index
            ind = noteView->currentIndex();

          QString notePath = noteModel->filePath(ind);
          if(!QFileInfo(notePath).exists())
          {
              QMessageBox::warning(this,tr("Note does not exist"), tr("The selected note cannot be opened because it has been moved or renamed!"));
              return;
          }

          openOneNote(notePath);
     }
}

void MainWindow::openOneNote(QString path)
{
     if(noteIsOpen(path))
     {
          return;
     }


     Note* note = new Note(path);
     openNotes += note;
     note->setObjectName(path);

     Note::addToOpenNoteList(path);
     if(QSettings().value("kinetic_scrolling", false).toBool())
     {
         QScroller::grabGesture(note->textEdit()->viewport(), QScroller::LeftMouseButtonGesture);
     }
     // only show the searchBar if the note contains the search text
     if(noteModel->sourceModel() == findNoteModel)
        note->setSearchBarText(searchText->text());
     note->showAfterLoaded();

     QStringList recentNoteList = QSettings().value("Recent_notes").toStringList();
     recentNoteList.removeAll(path); //remove if already present
     recentNoteList.prepend(path); //prepend to be at the first place
     while(recentNoteList.size() > QSettings().value("Number_of_recent_Notes",5).toInt())
       recentNoteList.removeLast(); //make sure to keep the size set in the preferences
     QSettings().setValue("Recent_notes", recentNoteList);
     createAndUpdateRecent(); //also calls createRecent
}

void MainWindow::createAndUpdateRecent()
{
     menu_Open_recent->clear();
     QStringList recentFilePaths = QSettings().value("Recent_notes").toStringList();

     //remove all nonexisting note paths
     for(int i = 0; i < recentFilePaths.size(); i++)
        if(!QFile(recentFilePaths[i]).exists())
          recentFilePaths.removeAll(recentFilePaths[i]);

     //remove all note paths to match the size in the preferences
     while(recentFilePaths.size() > QSettings().value("Number_of_recent_Notes",5).toInt())
       recentFilePaths.removeLast();

     fileWatcher->addPaths(recentFilePaths);
     QSettings().setValue("Recent_notes", recentFilePaths); //save for later

     menu_Open_recent->setDisabled(QSettings().value("Recent_notes").toStringList().isEmpty()); //disable if zero

     QAction *recentAction = 0;
     for(int i = 0; i < recentFilePaths.size(); i++)
     {
          recentAction = new QAction(menu_Open_recent);
          QString fileName = QFileInfo(recentFilePaths[i]).fileName(); //get file name
          recentAction->setText(fileName); //displayed name
          recentAction->setData(recentFilePaths[i]); //add the path as data
          connect(recentAction, SIGNAL(triggered()), this, SLOT(openRecent())); 
          menu_Open_recent->addAction(recentAction);
     }
}

void MainWindow::openRecent()
{
     QAction *action = qobject_cast<QAction *>(sender()); //get QAction that was the sender of the signal
     if(action){ //if not NULL
       if(noteIsOpen(action->data().toString()))
         return;
       else
         openOneNote(action->data().toString()); //open note with the path in data of QAction
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
        QScroller::grabGesture(textEdit->viewport(), QScroller::LeftMouseButtonGesture);
    }

    mainWindow->setCentralWidget(textEdit);
    TextSearchToolbar * searchBar = new TextSearchToolbar(textEdit,mainWindow);
    mainWindow->addToolBar(searchBar);
    mainWindow->resize(Note::editorSize());
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
            it = openNotes.erase(it); // set iterator to the item after the erased item

            if(it >= openNotes.end()) // iterator may be pointing to an element greater than the last element
                return 0;
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
     QString path = folderModel->rootPath() + QDir::separator() + tr("new notebook");
     int counter = 0;
     while(QDir(path).exists())
     {
         ++counter;
         path = folderModel->rootPath() + QDir::separator() + tr("new notebook (%1)").arg(QString::number(counter));
     }
     QModelIndex idx = folderModel->mkdir(folderView->rootIndex(),QDir(path).dirName());

     if(idx.isValid())
     {
         folderView->setCurrentIndex(idx);
         folderView->edit(idx); // 'open' for rename
     }
     folderModel->sort(0);
     folderView->scrollTo(folderView->selectionModel()->selectedRows().first());
}

void MainWindow::newNote(){
     QString filePath = noteModel->rootPath() + QDir::separator() + tr("new note");
     int counter = 0;
     while(QFile::exists(filePath))
     {
         ++counter;
         filePath = noteModel->rootPath() + QDir::separator() + tr("new note (%1)").arg(QString::number(counter));
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
     if(!noteView->selectionModel()->selectedRows().isEmpty())
     {
        noteView->scrollTo(noteView->selectionModel()->selectedRows().first());
     }
}

void MainWindow::renameFolder(){
     if(!folderView->selectionModel()->selectedRows().isEmpty())
       folderView->edit(folderView->selectionModel()->selectedRows().first());
     getToBerenamedNotebook = folderView->selectionModel()->selectedRows().first().data(Qt::DisplayRole).toString();
}

void MainWindow::renameNote(){
     if(!noteView->selectionModel()->selectedRows().isEmpty())
       noteView->edit(noteView->selectionModel()->selectedRows().first());
}

void MainWindow::removeFolder(){
     if(folderView->selectionModel()->selectedRows().isEmpty())
       return;

     QStringList dirList = QDir(QSettings().value("root_path").toString()).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

     // keep at least one folder
     if(dirList.size() == 1)
     {
        QMessageBox::information(this,tr("Notebook could not be deleted"),tr("The notebook could not be deleted because one notebook must remain"));
        return;
     }

     QModelIndex idx = folderView->selectionModel()->selectedRows().first();

     // remove empty folders without prompt else show a yes/abort message box
     if(!folderModel->rmdir(idx)) // folder not empty
     {
         if(QMessageBox::warning(this,tr("Delete Notebook"),
                                tr("Are you sure you want to delete the notebook \"%1\" and move all containing notes to the trash?").arg(folderModel->fileName(idx)),
                             QMessageBox::Yes | QMessageBox::Abort) != QMessageBox::Yes)
            return;


        // list all files
        QString path = folderModel->filePath(idx);
        const QStringList fileList = QDir(path).entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

        QModelIndexList indexes;
        for(const QString & fileName : fileList)
        {
            indexes << folderModel->index(QString("%1/%2").arg(path).arg(fileName));
        }

        folderModel->copyNotesToBackupDir(indexes);
        folderModel->removeList(indexes);

        // try to remove the (now empty?) folder again
        if(!folderModel->rmdir(idx))
        {
            QMessageBox::warning(this,tr("Notebook could not be deleted"), tr("The notebook could not be deleted because one or more notes inside the notebook could not be deleted."));
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
     if(noteView->selectionModel()->selectedRows().isEmpty())
       return;

     QString names;
     const auto list = noteModel->fileNames(noteView->selectionModel()->selectedRows());
     for(QString name : list)
          names += "\"" + name + "\"\n";

     QString title;
     QString text;

     int numNotes = noteView->selectionModel()->selectedRows().size();

     if(noteModel->allSizeZero(noteView->selectionModel()->selectedRows()))
     {
         // zero size files do not go to the trash
         title = tr("Deleting notes");
         text = tr("Are you sure you want to permanently delete the selected notes?");
     }
     else if(numNotes > 1)
     {
         title = tr("Delete Multiple Notes");
         text =  tr("Are you sure you want to move these %1 notes to the trash?\n\n%2").arg(QString::number(numNotes)).arg(names);
     }
     else
     {
         title = tr("Delete Note");
         text = tr("Are you sure you want to move the note %1 to the trash?").arg(names);
     }

     if(QMessageBox::warning(this,title,text,QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
       return;

     QModelIndexList selectedRows = noteView->selectionModel()->selectedRows();

     // try to copy the files to be removed into the backup folder
     noteModel->copyNotesToBackupDir(selectedRows);
     noteModel->removeList(selectedRows);
}

void MainWindow::setKineticScrollingEnabled(bool b)
{
    QList<QAbstractScrollArea* > widgets  = QList<QAbstractScrollArea*>() << noteView << folderView;
    for(QWidget * w : openNotes)
    {
        if(Note * note = qobject_cast<Note*>(w))
            widgets << note->textEdit();
    }

    if(b)
    {
        for(QAbstractScrollArea* widget : widgets)
            if(widget)
            {
                QScroller::grabGesture(widget->viewport(), QScroller::LeftMouseButtonGesture);
            }
    }
    else
    {
        for(QAbstractScrollArea* widget : widgets)
            if(widget)
            {
                QScroller::ungrabGesture(widget->viewport());
            }
    }
}

void MainWindow::showContextMenuFolder(const QPoint &pos){
     QPoint globalPos = folderView->mapToGlobal(pos);

     QMenu menu;

     if(!folderView->indexAt(pos).isValid()) // if index doesn't exists at position
     {
         menu.addAction(actionNew_folder);
     }
     if(folderView->indexAt(pos).isValid()) // if index exists at position
     {
         menu.addAction(actionRename_folder);
         menu.addAction(actionDelete_folder);
         if(!shortcutNoteList.isEmpty())
         {
            menu.addSeparator();
            menu.addAction(action_Paste);
         }
     }
     menu.exec(globalPos);
}

void MainWindow::showContextMenuNote(const QPoint &pos){
     QPoint globalPos = noteView->mapToGlobal(pos);

     QMenu menu;

     if(!noteView->indexAt(pos).isValid() &&
        !(noteModel->sourceModel() == findNoteModel)) // if index doesn't exists at position
     {
         menu.addAction(actionNew_note);
         menu.addSeparator();
         menu.addAction(action_Paste);
     }
     if(noteView->indexAt(pos).isValid()) // if index exists at position
     {
         if(noteView->selectionModel()->selectedRows().count() == 1)
         {
             menu.addAction(actionRename_note);
             actionDelete_note->setText(actionDelete_note->text());
         }
         else
         {
             QAction* openAll = new QAction(tr("&Open notes"), &menu);
             connect(openAll, SIGNAL(triggered()), this, SLOT(openAllNotes()));
             menu.addAction(openAll);
             actionDelete_note->setText(tr("&Delete notes"));
         }
         menu.addAction(actionDelete_note);

         // show source code menu entry
         if(QSettings().value("show_source").toBool() && noteView->selectionModel()->selectedRows().count() == 1)
         {
             QAction* showSourceAction = new QAction(tr("Show &Source"), &menu);
             connect(showSourceAction,SIGNAL(triggered()),this,SLOT(openNoteSource()));
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
     {
         const auto selected = noteView->selectionModel()->selectedRows();
       for(QModelIndex idx  : selected)
          shortcutNoteList << noteModel->filePath(idx);
     }

     if(!shortcutNoteList.isEmpty())
       action_Paste->setEnabled(true);

     if(noteIconProvider)
       delete noteIconProvider;
     noteIconProvider = new FileIconProvider();
     noteIconProvider->setCutFiles(shortcutNoteList);
     noteFSModel->setIconProvider(noteIconProvider);

     const auto selected = noteView->selectionModel()->selectedRows();
     for(QModelIndex idx : selected)
       noteView->update(idx);
}

void MainWindow::pasteFiles()
{
     if(shortcutNoteList.isEmpty())
       return;

     QString copyErrorFiles;
     for(QString note : shortcutNoteList)
     {
          if(!QFile(note).copy(folderModel->filePath(
             folderView->selectionModel()->selectedRows().first())
             + QDir::separator() + QFileInfo(note).fileName()))
                     copyErrorFiles += "\"" + note + "\"\n";
          else QFile(note).remove();
     }
     if(!copyErrorFiles.isEmpty())
       QMessageBox::critical(this, tr("Copy error"), tr("Notes of the same names "
                             "already exist in this notebook:\n\n%1").arg(QDir::toNativeSeparators(copyErrorFiles)));
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
     {
          shortcutNoteList.clear();

          if(noteIconProvider)
            delete noteIconProvider;
          noteIconProvider = new FileIconProvider();
          noteFSModel->setIconProvider(noteIconProvider);

          //update noteView's items
          QSet<QModelIndex> rows; //no duplicates for performance
          for(int i = 0; noteView->indexAt(QPoint(0,i)).isValid(); i++)
               rows << noteView->indexAt(QPoint(0,i));

          for(QModelIndex index : rows)
               noteView->update(index);
     }
}

void MainWindow::about()
{
   //Versioning
     QFile versionFile(":version");
     versionFile.open(QIODevice::ReadOnly | QIODevice::Text);
     QTextStream in(&versionFile);
     QString version = in.readLine();
     versionFile.close();

     QMessageBox::about(this, tr("About ") + qApp->applicationName(),
                      tr("<h1>%1 version %2</h1><p><b>%1</b> is a note taking application</p>"
                   "<p>Copyright © 2020 Christian Metscher</p>"

                   "<p>Permission is hereby granted, free of charge,"
                   " to any person obtaining a copy of this software and associated documentation files (the \"Software\"),"
                   " to deal in the Software without restriction,"
                   " including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,"
                   " and to permit persons to whom the Software is furnished to do so, subject to the following conditions:</p>"
                   "The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software."
                   "<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,"
                   " FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,"
                   " WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</p>"
                         ).arg(qApp->applicationName()).arg(version)  //: %1 is the application name, also do not translate the licence text

                        // these macros should work for every compiler
                        +   "</p><p>Qt " + QString(QT_VERSION_STR) + "</p>");  // the Qt version this build is linked against
}
