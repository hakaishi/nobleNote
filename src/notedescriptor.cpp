#include "notedescriptor.h"
#include <QFile>
#include <QTimer>
#include <QSettings>
#include "htmlnotereader.h"
#include "htmlnotewriter.h"
#include <QMessageBox>
#include <QFileInfo>
#include <QTextDocument>
#include <QScopedPointer>
#include <QDebug>

NoteDescriptor::NoteDescriptor(QString filePath, TextDocument *document, QWidget *noteWidget) :
    QObject(noteWidget), readOnly_(false)
{
    Activity = ProcessEvents; // this will block focusInEvent from the textEdit if this object is constructed

    noteWidget_ = noteWidget;
    document_ = document;
    filePath_ = filePath;
    load(filePath_);
    connect(document_,SIGNAL(delayedModificationChanged()),this,SLOT(stateChange()));

    QTimer::singleShot(0,this,SLOT(setActivityIdle())); // enable stateChange() after all events have been processed
}

void NoteDescriptor::stateChange()
{
    if(Activity !=Idle || readOnly_)
        return;

    Activity = CheckFilePath;

    // get file Path
    // uuid_ != reader.uuid() checks if the file has been replaces by another file of the same name
    if(!QFile::exists(filePath_) || uuid_ != HtmlNoteReader::uuid(filePath_))
    {
        // search the moved or renamed file by its uuid
        QString newFilePath = HtmlNoteReader::findUuid(uuid_, QSettings().value("rootPath").toString());

        if(newFilePath.isEmpty())
        {
            if(QMessageBox::warning(noteWidget_,tr("Note does not longer exist"),
                                    tr("The note has been removed by another program. Should the note be kept open?"),
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
                save(filePath_,uuid_); // save under old path with new uuid
                Activity = Idle;
            }
            else
            {
                emit close();
            }
            return;
        }
        filePath_ = newFilePath; // old filePath_ not longer needed
    }

     HtmlNoteReader reader(filePath_);


    Activity = CheckLastChange;
    if(lastChange_ < reader.lastChange() && !reader.lastChange().isNull()) // modified elsewhere, lastChange can be null for html files not created with this software
    {
        if(document_->isModified() && QMessageBox::warning(noteWidget_,tr("Note modified"),
                                                           tr("The note has been modified by another program. Should the note be saved under a different name?"
                                                              " Else the note will be reloaded."),
                                                           QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
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
            Activity = Idle;
        }
        else // not modified, silently reload
        {
            load(filePath_);
            Activity = Idle;
        }
        return;
    }

    if(document_->isModified())
    {
        save(filePath_,uuid_);
        document_->setModified(false);
        Activity = Idle;
        return;
    }

    Activity = Idle;
}

void NoteDescriptor::setActivityIdle()
{
    Activity = Idle;
}

void NoteDescriptor::save(const QString& filePath,QUuid uuid)
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

     if(noteWidget_)
         noteWidget_->setWindowTitle(title_);
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

    // TODO automatically write html notes with missing uuid/datetimes or xml notes to html into the same filename
    // Important: must destroy reader objects before writing

    if(noteWidget_)
        noteWidget_->setWindowTitle(title_);

    // dates can be null, HtmlNoteWriter will generate non null dates
    lastChange_ = reader->lastChange();
    createDate_ = reader->createDate();
    uuid_ = reader->uuid(); // can be null

     // incomplete note, overwrite with html format
    if(isXmlNote || lastChange_.isNull() /*|| createDate_.isNull()*/ || uuid_.isNull())
    {
        uuid_ = uuid_.isNull() ? QUuid::createUuid() : uuid_;
        //lastChange_ gets written by save
        // createDate_ gets written by HtmlNoteWriter
        readOnly_ = !QSettings().value("convert_notes",true).toBool();
        if(!readOnly_)
            save(filePath_,uuid_); // only overwrite if convert_notes is disabled in settings
    }


    //lastMetadataChange_ = reader->lastMetadataChange().isNull() ? QFileInfo(filePath).lastModified() : reader->lastMetadataChange();

    document_->setModified(false); // avoid emit of delayedModificationChanged()
    delete reader;
}
