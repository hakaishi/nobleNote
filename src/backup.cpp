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

#include "backup.h"
#include "treemodel.h"
#include <QFile>
#include <QDirIterator>
#include <QSettings>

Backup::Backup(QWidget *parent): QDialog(parent){
     setupUi(this);

     splitter = new QSplitter(groupBox);
     gridLayout_2->addWidget(splitter);
     treeView = new QTreeView(splitter);
     frame = new QFrame(splitter);
     gridLayout3 = new QGridLayout(frame);
     label = new QLabel(frame);
     label->setText(tr("Preview of the selected backup"));
     gridLayout3->addWidget(label, 0, 0, 1, 1);
     textEdit = new QTextEdit(this);
     textEdit->setDisabled(frame);
     gridLayout3->addWidget(textEdit, 1, 0, 1, 1);

     QStringList headers;
     headers << tr("Backups") << tr("Date");

     QString path = QSettings().value("backupDirPath").toString();
     QStringList files;
     QDirIterator it(path, QDirIterator::Subdirectories);
     while(it.hasNext())
     {
         QString filePath = it.next();
         if(it.fileInfo().isFile())
            files << filePath;
     }

     textDocument = new QTextDocument(this);
     textEdit->setDocument(textDocument);

     model = new TreeModel(headers, files, textDocument);
     treeView->setModel(model);

     for(int column = 0; column < model->columnCount(); ++column)
        treeView->resizeColumnToContents(column);

     treeView->setAlternatingRowColors(true);
     treeView->setSelectionBehavior(QAbstractItemView::SelectItems);
     treeView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
     treeView->setSortingEnabled(true);

     //TODO: should be selectionChanged instead of activated...
     connect(treeView, SIGNAL(activated(QModelIndex)), this, SLOT(showPreview(QModelIndex)));
}

void Backup::showPreview(const QModelIndex &idx){
     QString str = idx.data().toString();
     QString text;
     foreach(QString s, model->contents)
       if(s.contains(str + "\t"))
         text = s.remove(str + "\t");
     textEdit->setText(text);
}
