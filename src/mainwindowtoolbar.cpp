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

#include "mainwindowtoolbar.h"
#include <QItemSelection>

MainWindowToolbar::MainWindowToolbar(QWidget *parent) : QToolBar(parent) {

    setWindowTitle(tr("Main window toolbar"));
    setObjectName(tr("Mainwindowtoolbar"));
    setToolButtonStyle(Qt::ToolButtonIconOnly);

    newFolderAction = new QAction(QIcon(":newFolder"),
      tr("Create new folder"), this);
    newFolderAction->setPriority(QAction::LowPriority);
    newFolderAction->setShortcut(Qt::CTRL + Qt::Key_F);
    addAction(newFolderAction);

    removeFolderAction = new QAction(QIcon(":deleteFolder"),
      tr("Delete folder"), this);
    newFolderAction->setPriority(QAction::LowPriority);
    newFolderAction->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_F);
    addAction(removeFolderAction);

    renameFolderAction = new QAction(QIcon(":renameFolder"),
      tr("Rename folder"), this);
    newFolderAction->setPriority(QAction::LowPriority);
    addAction(renameFolderAction);

    folderActions << newFolderAction << removeFolderAction << renameFolderAction;

    addSeparator();

    newNoteAction = new QAction(QIcon(":newNote"),
      tr("Create new note"), this);
    newNoteAction->setPriority(QAction::LowPriority);
    newNoteAction->setShortcut(Qt::CTRL + Qt::Key_N);
    addAction(newNoteAction);

    removeNoteAction = new QAction(QIcon(":deleteNote"),
      tr("Delete note"), this);
    newNoteAction->setPriority(QAction::LowPriority);
    newNoteAction->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_N);
    addAction(removeNoteAction);

    renameNoteAction = new QAction(QIcon(":renameNote"),
      tr("Rename note"), this);
    newNoteAction->setPriority(QAction::LowPriority);
    addAction(renameNoteAction);

    noteActions << removeNoteAction << renameNoteAction;
}

void MainWindowToolbar::onFolderSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
        foreach(QAction * action, folderActions)
            action->setEnabled(!selected.isEmpty());
}

void MainWindowToolbar::onNoteSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    foreach(QAction * action, noteActions)
        action->setEnabled(!selected.isEmpty());
}
