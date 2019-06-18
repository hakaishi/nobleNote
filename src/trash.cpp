/* nobleNote, a note taking application
 * Copyright (C) 2019 Christian Metscher <hakaishi@web.de>,
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

#include "trash.h"
#include <QSettings>
#include <QMessageBox>

Trash::Trash(QHash<QString,QStringList> *backupDataHash, QWidget *parent): QDialog(parent){
     setupUi(this);

     setAttribute(Qt::WA_DeleteOnClose);

     const auto keys =  backupDataHash->keys();
     for(QString key : keys)
     {
          QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);
          item->setText(0,backupDataHash->value(key).first()); //title
          item->setData(0,Qt::UserRole,backupDataHash->value(key)); //title, path and content
     }

     treeWidget->sortByColumn(0,Qt::AscendingOrder);
     // TODO flickcharm here

     connect(treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(showPreview()));
     connect(deleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteBackup()));
     connect(restoreButton, SIGNAL(clicked(bool)), this, SLOT(restoreBackup()));
}

void Trash::restoreBackup()
{
     if(treeWidget->selectedItems().isEmpty())
       return;

     QString dir = QSettings().value("root_path").toString()+"/restored notes";
     if(!QDir(dir).exists())
       QDir().mkpath(dir);

     for(QTreeWidgetItem *item : treeWidget->selectedItems())
     {
          QStringList dataList = item->data(0,Qt::UserRole).toStringList();
          QString title = dataList.takeFirst();
          if(!QFile(dataList.first()).exists())
            return;
          else
          {
             QString filePath = dir+QDir::separator()+title;
             int i = 0;
             while(QFile::exists(filePath))
             {
                i++;
                filePath = dir+QDir::separator()+title+" ("+QString::number(i)+")";
             }
             QFile(dataList.first()).copy(filePath);
          }
          delete item;
     }
}

void Trash::deleteBackup()
{
     if(treeWidget->selectedItems().isEmpty())
        return;

     QStringList files;
     const QList<QTreeWidgetItem*> itemList = treeWidget->selectedItems();
     for(QTreeWidgetItem *item : itemList)
     {
          QStringList dataList = item->data(0,Qt::UserRole).toStringList();
          dataList.takeFirst(); //removing title from the list
          files << dataList.first();
     }

     if(QMessageBox::warning(this,tr("Deleting notes"),
          tr("Are you sure you want to permanently delete the selected notes?")
             ,QMessageBox::Yes | QMessageBox::Abort) != QMessageBox::Yes)
        return;

     for(QString file : files)
     {
          if(QFile(file).exists())
            QFile(file).remove();

          QString uuid = file;
          uuid.remove(QSettings().value("backup_dir_path").toString() + QDir::separator());
          QSettings().remove("Notes/" + QUuid(uuid).toString() + "_size");
          QSettings().remove("Notes/" + QUuid(uuid).toString() + "_cursor_position");
          QSettings().remove("Notes/" + QUuid(uuid).toString() + "_window_position");
     }

     for(QTreeWidgetItem *item : itemList)
          delete item;
}

void Trash::showPreview()
{
     if(treeWidget->currentItem() == NULL) //Prevents program crush
     {
          textEdit->clear();
          return;
     }

     if(!treeWidget->currentItem()->isSelected())
     {
          if(treeWidget->selectedItems().count() != 1)
             textEdit->clear();
          else
             textEdit->setText(treeWidget->selectedItems().first()->data(0,Qt::UserRole).toStringList().last());
     }
     else
        textEdit->setText(treeWidget->currentItem()->data(0,Qt::UserRole).toStringList().last());
}
