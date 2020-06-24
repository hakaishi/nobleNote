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

#include "systemtraycreator.h"
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QListIterator>
#include <QMenu>

SystemTrayCreator::SystemTrayCreator(QObject *parent) : QObject(parent)
{

}

void SystemTrayCreator::populateMenu(QMenu * menu)
{

    menu->clear();

    QString rootPath = QSettings().value("root_path").toString();


    QListIterator<QFileInfo> it(QDir(rootPath).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks,
                                                             QDir::Name | QDir::IgnoreCase));
    while (it.hasNext()) {

        QFileInfo nextThing = it.next();

        QDir nextDir(nextThing.filePath());

        QMenu* notebookMenu = menu->addMenu(nextDir.dirName());

        QFileInfoList entries = nextDir.entryInfoList(QDir::Files,QDir::Name | QDir::IgnoreCase);

        QListIterator<QFileInfo> sit(entries);

        while (sit.hasNext()) {
            QFileInfo nextNote = sit.next();
            QAction* action = notebookMenu->addAction(nextNote.fileName());
            connect(action,&QAction::triggered,menu,[this,nextNote](){ this->noteClicked(nextNote.filePath());});
        }

    }
}
