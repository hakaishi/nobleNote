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

#include "noteimporter.h"
#include <QFileDialog>
#include <QSettings>
#include <QWidget>
#include <QtConcurrentMap>

NoteImporter::NoteImporter(QObject *parent) :
    QObject(parent)
{
    parentWidget = this->parent()->isWidgetType() ? qobject_cast<QWidget*>(this->parent()) : 0;
}

void NoteImporter::importDialog()
{
     if(fileDialog)
       return;

     importFiles.clear(); //remove old files

     fileDialog = new QFileDialog(parentWidget, tr("Select one or more tomboy or gnote notes"),
                      QSettings().value("import_path").toString(), tr("Notes")+"(*.note)");
     fileDialog->setViewMode(QFileDialog::Detail);
     fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
     fileDialog->setFileMode(QFileDialog::ExistingFiles);
     fileDialog->setOption(QFileDialog::ReadOnly);
     fileDialog->show();
     QObject::connect(fileDialog, SIGNAL(accepted()), this, SLOT(importXmlNotes()));
     QObject::connect(fileDialog, SIGNAL(rejected()), fileDialog, SLOT(deleteLater()));
}

void NoteImporter::importXmlNotes()
{
     importFiles = fileDialog->selectedFiles();
     if(importFiles.isEmpty())
        return;

     QSettings().setValue("import_path",QFileInfo(importFiles.last()).absolutePath());

     dialog = new QProgressDialog(parentWidget);
     dialog->setLabelText(QString(tr("Importing notes...")));

     progressReceiver = new ProgressReceiver(parentWidget);
     xml2HtmlFunctor.path = QSettings().value("root_path").toString();
     xml2HtmlFunctor.p = progressReceiver;

     futureWatcher = new QFutureWatcher<void>(parentWidget);
     futureWatcher->setFuture(QtConcurrent::map(importFiles, xml2HtmlFunctor));

     QObject::connect(progressReceiver,SIGNAL(valueChanged(int)),dialog, SLOT(setValue(int)));
     QObject::connect(futureWatcher, SIGNAL(finished()), dialog, SLOT(reset()));
     QObject::connect(dialog, SIGNAL(canceled()), futureWatcher, SLOT(cancel()));

     const auto objects = QList<QObject*>()  << futureWatcher << dialog << progressReceiver << fileDialog;

     for(QObject * o : objects)
     {
         connect(futureWatcher, SIGNAL(canceled()),o,SLOT(deleteLater()));
         connect(futureWatcher, SIGNAL(finished()),o,SLOT(deleteLater()));
     }

//     QObject::connect(futureWatcher, SIGNAL(canceled()), futureWatcher, SLOT(deleteLater()));
//     QObject::connect(futureWatcher, SIGNAL(finished()), futureWatcher, SLOT(deleteLater()));
//     QObject::connect(futureWatcher, SIGNAL(canceled()), dialog, SLOT(deleteLater()));
//     QObject::connect(futureWatcher, SIGNAL(finished()), dialog, SLOT(deleteLater()));
//     QObject::connect(futureWatcher, SIGNAL(canceled()), progressReceiver, SLOT(deleteLater()));
//     QObject::connect(futureWatcher, SIGNAL(finished()), progressReceiver, SLOT(deleteLater()));
//     QObject::connect(futureWatcher, SIGNAL(canceled()), fileDialog, SLOT(deleteLater()));
//     QObject::connect(futureWatcher, SIGNAL(finished()), fileDialog, SLOT(deleteLater()));

     dialog->show();
}

