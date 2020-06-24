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
#include <QtConcurrentRun>

NoteDescriptor::NoteDescriptor(QString filePath,QTextBrowser * textBrowser, TextDocument *document, QWidget *noteWidget) :
    QObject(noteWidget), readOnly_(false)
{
    initialLock = new Lock; // this will block focusInEvent from the textEdit from signalling stateChange() if this object is constructed

    noteWidget_ = noteWidget;
    document_ = document;
    textBrowser_ = textBrowser;
    filePath_ = filePath;
    QTimer::singleShot(0,this,SLOT(load())); // load after gui events have been processed
    connect(document_,SIGNAL(delayedModificationChanged()),this,SLOT(stateChange()));
    connect(this,SIGNAL(loadFinished(HtmlNoteReader*)),this,SLOT(onHtmlLoadFinished(HtmlNoteReader*)),Qt::QueuedConnection); // signal across threads, used to load html asynchronous
    // unlocking stateChange happens in onLoadFinished, which is called by load();
}

/**
 * @brief NoteDescriptor::stateChange
 *
 * 1. checks if the file exists and check if its uuid is still the same
 * 2. rare case: check for modification inside the autosave-interval
 * 3. reload if modified
 */
void NoteDescriptor::stateChange()
{
    if(Lock::isLocked()|| readOnly_)
        return;

    Lock lock;

    /// 1.
    // an filePath_ that still exists and an uuid that has changed means the file has been replaced by another file of the same name
    if(QFile::exists(filePath_))
    {
        QUuid uuid = HtmlNoteReader::uuid(filePath_);
        if(!uuid.isNull() && uuid_ != uuid)
        {
            findOrReCreate();
        }
    }
    else // file does not longer exist
    {
        findOrReCreate();
    }

    /// 2.
     // rare: reload a note if it has been modified before autosave has been triggered
    if(lastChange_ < QFileInfo(filePath_).lastModified())
    {
        if(document_->isModified() && QMessageBox::warning(noteWidget_,tr("Note modified"),
               tr("This note has been modified by another instance of %1. Should the"
                 " note be saved under a different name? Else the note will be reloaded.").arg(
                 qApp->applicationName()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            uuid_ = QUuid::createUuid();
            title_ = QFileInfo(filePath_).baseName();
            createDate_ = QDateTime::currentDateTime();

            save(filePath_,uuid_,false); // save under new name with new uuid
        }
        else // not modified, silently reload
        {
            load();
        }
        return;
     }

     /// 3.
    // reload if modified
    if(document_->isModified())
    {
        save(filePath_,uuid_,true);
        document_->setModified(false);
        return;
    }
}

void NoteDescriptor::findOrReCreate()
{
    // search the moved or renamed file by its uuid
    QString newFilePath = HtmlNoteReader::findUuid(uuid_, QSettings().value("root_path").toString());

    if(newFilePath.isEmpty())
    {
        if(QMessageBox::warning(noteWidget_,tr("Note does not exist"),
                                tr("This note does not longer exist. Do you want to keep the editor open?"),
                                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            title_ = QFileInfo(filePath_).baseName();
            if(noteWidget_)
                noteWidget_->setWindowTitle(title_); // update window title
            save(filePath_,uuid_,false); // save under old path with new uuid
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

void NoteDescriptor::unlockStateChange()
{
    delete initialLock;
}



void NoteDescriptor::save(const QString& filePath,QUuid uuid, bool overwriteExisting)
{
    if(!overwriteExisting)
    {
        // avoid overwriting an existing file (this case might arise when the user clicks a dialog to save a not longer existing note)
        int counter = 0;
        QString origPath = filePath_;
        while(QFile::exists(filePath_))
        {
            ++counter;
            filePath_ = origPath +  QString(" (%1)").arg(counter);
        }
    }

    if(!QDir(QFileInfo(filePath).absolutePath()).exists())
        QDir().mkpath(QFileInfo(filePath).absolutePath());

    if(!QDir(QSettings().value("backup_dir_path").toString()).exists())
        QDir().mkpath(QSettings().value("backup_dir_path").toString());

    write(filePath,uuid); // write note
    QString backup_dir_path = QSettings().value("backup_dir_path").toString();
    QString uuidStr = uuid.toString();
    uuidStr.chop(1); // }
    uuidStr = uuidStr.remove(0,1); // {
    QString backupFilePath = backup_dir_path + QDir::separator() + uuidStr;
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

void NoteDescriptor::load()
{
    AbstractNoteReader * reader;
    // check if the file is a tomboy note

    if(XmlNoteReader::mightBeXmlNote(filePath_))
    {
        reader = new XmlNoteReader(filePath_,document_);
        reader->read(); // XmlNoteReader.read can only be run in the gui thread

        onLoadFinished(reader);
    }
    else
    {
        reader = new HtmlNoteReader(filePath_);

        // run read concurrently
         // calls onHtmlLoadFinished to set the text document in the gui thread, then calls onLoadFinished
        QtConcurrent::run(this,&NoteDescriptor::loadHtml,reader);

    }
    title_ = QFileInfo(filePath_).baseName();
}

void NoteDescriptor::loadHtml(AbstractNoteReader *reader)
{
    reader->read();
    emit loadFinished(static_cast<HtmlNoteReader*>(reader));
}

// wrapper, because document must be set in ui thread
void NoteDescriptor::onHtmlLoadFinished(HtmlNoteReader *reader)
{
    // this call is expensive but cannot moved out of the gui thread because QTextDocument is a QObject which has thread affinity for the
    // thread is has been created in
    document_->setHtml(reader->html());
    onLoadFinished(reader);
}

void NoteDescriptor::onLoadFinished(AbstractNoteReader *reader, bool isXmlNote)
{
    if(noteWidget_)
        noteWidget_->setWindowTitle(title_);

    // dates can be null, HtmlNoteWriter will generate non null dates
    createDate_ = reader->createDate();
    lastChange_ = QFileInfo(filePath_).lastModified();
    uuid_ = reader->uuid(); // can be null

    QTextCursor cursor(document_);
    cursor.setPosition(QSettings().value("Notes/"+uuid_.toString()+"_cursor_position").toInt());
    textBrowser_->setTextCursor(cursor);

    delete reader;
    reader = 0;


     // incomplete note, overwrite with html format
    if(isXmlNote /*|| createDate_.isNull()*/ || uuid_.isNull())
    {
        uuid_ = uuid_.isNull() ? QUuid::createUuid() : uuid_;
        //lastChange_ gets written by save
        // createDate_ gets written by HtmlNoteWriter
        readOnly_ = !QSettings().value("convert_notes",true).toBool();
        if(!readOnly_)
            save(filePath_,uuid_,true); // only overwrite if convert_notes is enabled in settings
    }


    //lastMetadataChange_ = reader->lastMetadataChange().isNull() ? QFileInfo(filePath).lastModified() : reader->lastMetadataChange();

    document_->setModified(false); // avoid emit of delayedModificationChanged()

     QTimer::singleShot(0,this,SLOT(unlockStateChange())); // enable stateChange() after all events have been processed

}




bool NoteDescriptor::Lock::isLocked()
{
    return count >0;
}

int NoteDescriptor::Lock::count = 0;
