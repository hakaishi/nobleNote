#include "notedescriptor.h"
#include <QFile>
#include <QTimer>
#include <QSettings>

NoteDescriptor::NoteDescriptor(QString filePath, TextDocument *document, QObject *parent) :
    QObject(parent)
{
    stateChangeEnabled = false; // this will block focusInEvent from the textEdit if this object is constructed
    filePath_ = filePath;
    document_ = document;

    XmlNoteReader reader(filePath);
    reader.setFrame(document->rootFrame());
    reader.read(); // add the read text to the document

    uuid_ = reader.uuid().isNull() ? QUuid::createUuid() : reader.uuid();


    // dates can be null, XmlNoteWriter will generate non null dates
    lastChange_ = reader.lastChange();
    createDate_ = reader.createDate();
    lastMetadataChange_ = reader.lastMetadataChange();

    document_->setModified(false); // avoid emit of delayedModificationChanged()
    connect(document_,SIGNAL(delayedModificationChanged()),this,SLOT(stateChange()));

    QTimer::singleShot(0,this,SLOT(setStateChangeEnabled())); // enable stateChange() after all events have been processed
}

void NoteDescriptor::stateChange()
{
    if(!stateChangeEnabled)
        return;

    qDebug("stateChange()");

    // get file Path
    QString newFilePath = XmlNoteReader::findUuid(uuid_, QSettings().value("rootPath").toString());

    if(newFilePath.isEmpty())
    {
        // TODO handle missing file, old file path is in filePath_
        return;
    }
    filePath_ = newFilePath; // old filePath_ not longer needed

    QDateTime laterLastChange;
    {
        XmlNoteReader reader(newFilePath);
        reader.read();
        laterLastChange = reader.lastChange(); // TODO handle last change is null
    }

    if(this->lastChange_ < laterLastChange)
    {
         // TODO handle later lastChange date
        //if(document_->isModified())

        // else // not modified, silently reload
        //  reload();
        return;
    }

    if(document_->isModified())
    {
        save();
        document_->setModified(false);
        return;
    }
}

void NoteDescriptor::setStateChangeEnabled()
{
    stateChangeEnabled = true;
}

void NoteDescriptor::save()
{
    // HtmlNoteWriter writer(filePath_);
//    XmlNoteWriter writer(filePath_);
//    writer.setFrame(document_->rootFrame());
//    writer.setUuid(uuid_);
//    writer.setLastChange(QDateTime::currentDateTime());
//    writer.setLastMetadataChange(lastMetadataChange_);
//    writer.setCreateDate(createDate_);
//    writer.write();
}

void NoteDescriptor::load()
{
    // TODO title
}
