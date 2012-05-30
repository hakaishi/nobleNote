#include "notedescriptor.h"
#include <QFile>
#include <QTimer>
#include <QSettings>
#include "htmlnotereader.h"
#include "htmlnotewriter.h"
#include <QMessageBox>
#include <QDebug>

NoteDescriptor::NoteDescriptor(QString filePath, TextDocument *document, QWidget *noteWidget) :
    QObject(noteWidget)
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
    if(Activity !=Idle)
        return;

    qDebug("stateChange()");

    Activity = CheckFilePath;
    // get file Path
    QString newFilePath = HtmlNoteReader::findUuid(uuid_, QSettings().value("rootPath").toString());

    if(newFilePath.isEmpty())
    {
        if(QMessageBox::warning(noteWidget_,tr("Note does not longer exist"),
                               tr("The note has been removed by another program. Should the note be kept open?"),
                            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            save(filePath_,QUuid::createUuid()); // save under old path with new uuid
            Activity = Idle;
        }
        else
        {
            emit close();
        }
        return;
    }
    filePath_ = newFilePath; // old filePath_ not longer needed

    Activity = CheckLastChange;
    QDateTime laterLastChange;

    {
        HtmlNoteReader reader(filePath_);
        laterLastChange = reader.lastChange();
        if(this->lastChange_ < laterLastChange)
        {
            if(document_->isModified() && QMessageBox::warning(noteWidget_,tr("Note does not longer exist"),
                                                               tr("The note has been modified by another program. Should the note be saved under a different name?"
                                                                  "Else the note will be reloaded."),
                                                            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
                {
                    save(filePath_ + QUuid::createUuid().toString().mid(1,4),QUuid::createUuid()); // save under random name with new uuid
                }
            else // not modified, silently reload
            {
                load(filePath_);
                Activity = Idle;
            }
            return;
        }

    }

    if(document_->isModified())
    {
        save(filePath_,uuid_);
        document_->setModified(false);
        Activity = Idle;
        return;
    }
}

void NoteDescriptor::setActivityIdle()
{
    Activity = Idle;
}

void NoteDescriptor::save(const QString& filePath,QUuid uuid)
{
     HtmlNoteWriter writer(filePath,document_);
     writer.setUuid(uuid);
     lastChange_ = QDateTime::currentDateTime();
     writer.setLastChange(lastChange_);
     writer.setLastMetadataChange(lastMetadataChange_);
     writer.setCreateDate(createDate_);
     writer.write();
}

void NoteDescriptor::load(const QString& filePath)
{
    // TODO title

    HtmlNoteReader reader(filePath,document_);

    uuid_ = reader.uuid().isNull() ? QUuid::createUuid() : reader.uuid();


    // dates can be null, XmlNoteWriter will generate non null dates
    lastChange_ = reader.lastChange();
    createDate_ = reader.createDate();
    lastMetadataChange_ = reader.lastMetadataChange();

    document_->setModified(false); // avoid emit of delayedModificationChanged()
}
