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

#include "notedescriptor.h"
#include "htmlnotereader.h"
#include "htmlnotewriter.h"
#include <QFile>
#include <QTimer>
#include <QSettings>
#include <QMessageBox>
#include <QFileInfo>
#include <QTextDocument>
#include <QDir>
#include <QTextStream>

NoteDescriptor::NoteDescriptor(QString filePath,QTextBrowser * textBrowser, TextDocument *document, QWidget *noteWidget) :
    QObject(noteWidget), readOnly_(false)
{
    initialLock = new Lock; // this will block focusInEvent from the textEdit from signalling stateChange() if this object is constructed

    noteWidget_ = noteWidget;
    document_ = document;
    textBrowser_ = textBrowser;
    filePath_ = filePath;
    load(filePath_);
    connect(document_,SIGNAL(delayedModificationChanged()),this,SLOT(stateChange()));

    QTimer::singleShot(0,this,SLOT(unlockStateChange())); // enable stateChange() after all events have been processed
}

void NoteDescriptor::stateChange()
{
    if(Lock::isLocked()|| readOnly_)
        return;

    Lock lock;

    // get file Path
    // uuid_ != reader.uuid() checks if the file has been replaces by another file of the same name
    if(!QFile::exists(filePath_) || uuid_ != HtmlNoteReader::uuid(filePath_))
    {
        // search the moved or renamed file by its uuid
        QString newFilePath = HtmlNoteReader::findUuid(uuid_, QSettings().value("root_path").toString());

        if(newFilePath.isEmpty())
        {
            if(QMessageBox::warning(noteWidget_,tr("Keep non existing note"),
                                    tr("This note doesn't exist anymore. Keep this file open?"),
                                    QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
            {
                // check if file already exists
                uuid_ = QUuid::createUuid();
                int counter = 0;
                QString origPath = filePath_;
                while(QFile::exists(filePath_))
                {
                    ++counter;
                    filePath_ = origPath +  QString(" (%1)").arg(counter);
                }
                title_ = QFileInfo(filePath_).baseName();
                if(noteWidget_)
                    noteWidget_->setWindowTitle(title_); // update window title
                save(filePath_,uuid_); // save under old path with new uuid
            }
            else
            {
                emit close();
            }
            return;
        }
        filePath_ = newFilePath; // old filePath_ not longer needed
        title_ = QFileInfo(filePath_).baseName();
        if(noteWidget_)
            noteWidget_->setWindowTitle(title_); // update window title
    }

     HtmlNoteReader reader(filePath_);

    if(lastChange_ < reader.lastChange() && !reader.lastChange().isNull()) // modified elsewhere, lastChange can be null for html files not created with this software
    {
        if(document_->isModified() && QMessageBox::warning(noteWidget_,tr("Note modified"),
               tr("This note has been modified by another instance of %1. Should the"
                 " note be saved under a different name? Else the note will be reloaded.").arg(
                 qApp->applicationName()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            uuid_ = QUuid::createUuid();
            title_ = QFileInfo(filePath_).baseName();
            createDate_ = QDateTime::currentDateTime();

            // check if file already exists
            int counter = 0;
            QString origPath = filePath_;
            while(QFile::exists(filePath_))
            {
                ++counter;
                filePath_ = origPath +  QString(" (%1)").arg(counter);
            }
            save(filePath_,uuid_); // save under new name with new uuid
        }
        else // not modified, silently reload
        {
            load(filePath_);
        }
        return;
     }

    if(document_->isModified())
    {
        save(filePath_,uuid_);
        document_->setModified(false);
        return;
    }
}

void NoteDescriptor::unlockStateChange()
{
    delete initialLock;
}

void NoteDescriptor::save(const QString& filePath,QUuid uuid)
{
    if(!QDir(QFileInfo(filePath).absolutePath()).exists())
        QDir().mkpath(QFileInfo(filePath).absolutePath());

    if(!QDir(QSettings().value("backup_dir_path").toString()).exists())
        QDir().mkpath(QSettings().value("backup_dir_path").toString());

    write(filePath,uuid); // write note
    QString backup_dir_path = QSettings().value("backup_dir_path").toString();
    QString uuidStr = uuid.toString();
    uuidStr.chop(1); // }
    uuidStr = uuidStr.remove(0,1); // {
    QString backupFilePath = backup_dir_path + "/" + uuidStr;
    write(backupFilePath,uuid); // write backup

     if(noteWidget_)
         noteWidget_->setWindowTitle(title_);
}

void NoteDescriptor::write(const QString &filePath, QUuid uuid)
{
    HtmlNoteWriter writer(filePath);
    writer.setDocument(document_);
    // TODO uuid null?
    writer.setUuid(uuid);
    lastChange_ = QDateTime::currentDateTime();
    writer.setLastChange(lastChange_);
    //writer.setLastMetadataChange(lastMetadataChange_);
    writer.setCreateDate(createDate_);
    writer.setTitle(title_);

    writer.write();
}

void NoteDescriptor::load(const QString& filePath)
{
    AbstractNoteReader * reader;
    // check if the file is a tomboy note

    bool isXmlNote = false;


    if(XmlNoteReader::mightBeXmlNote(filePath))
    {
        reader = new XmlNoteReader(filePath,document_);
        title_ = reader->title();
        isXmlNote = true;
    }
    else
    {
        reader = new HtmlNoteReader(filePath,document_);
        title_ = QFileInfo(filePath).baseName();
    }


    if(noteWidget_)
        noteWidget_->setWindowTitle(title_);

    // dates can be null, HtmlNoteWriter will generate non null dates
    lastChange_ = reader->lastChange();
    createDate_ = reader->createDate();
    uuid_ = reader->uuid(); // can be null

    QTextCursor cursor(document_);
    cursor.setPosition(QSettings().value("Notes/"+uuid_+"_cursor_position").toInt());
    textBrowser_->setTextCursor(cursor);

    delete reader;
    reader = 0;

     // incomplete note, overwrite with html format
    if(isXmlNote || lastChange_.isNull() /*|| createDate_.isNull()*/ || uuid_.isNull())
    {
        uuid_ = uuid_.isNull() ? QUuid::createUuid() : uuid_;
        //lastChange_ gets written by save
        // createDate_ gets written by HtmlNoteWriter
        readOnly_ = !QSettings().value("convert_notes",true).toBool();
        if(!readOnly_)
            save(filePath_,uuid_); // only overwrite if convert_notes is enabled in settings
    }


    //lastMetadataChange_ = reader->lastMetadataChange().isNull() ? QFileInfo(filePath).lastModified() : reader->lastMetadataChange();

    document_->setModified(false); // avoid emit of delayedModificationChanged()
}

//void NoteDescriptor::showSource()
//{
//    stateChange(); // save current state
//    QFile file(filePath_);
//    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
//    {
//        qDebug("NoteDescriptor::toggleSource failed : could not open filepath");
//           return;
//    }
//    QString html;
//    QTextStream in(&file);
//    html = in.readAll();
//    file.close();
//    document_->setPlainText(html);
//    document_->setModified(false);
//}



bool NoteDescriptor::Lock::isLocked()
{
    return count >0;
}

int NoteDescriptor::Lock::count = 0;
