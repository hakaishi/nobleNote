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

MainWindowToolbar::MainWindowToolbar(QMainWindow *parent){

    setWindowTitle(tr("Main window toolbar"));
    setObjectName(tr("Mainwindowtoolbar"));
    setToolButtonStyle(Qt::ToolButtonIconOnly);

    newFolderAction = new QAction(QIcon::fromTheme("folder-new",
      QIcon(":newFolder")), tr("Create new &folder"), this);
    newFolderAction->setPriority(QAction::LowPriority);
    newFolderAction->setShortcut(Qt::CTRL + Qt::Key_F);
    addAction(newFolderAction);

    newNoteAction = new QAction(QIcon::fromTheme("filenew",
      QIcon(":newNote")), tr("Create new &note"), this);
    newNoteAction->setPriority(QAction::LowPriority);
    newNoteAction->setShortcut(Qt::CTRL + Qt::Key_N);
    addAction(newNoteAction);
}
