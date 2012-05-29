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

    uuid_ = reader.uuid();
    if(uuid_.isNull())
    {
        // should overwrite the file in this program's format with a generated uuid
        qDebug("NoteDescriptor::NoteDescriptor : uuid is null, TODO missing implementation of uuid generation");
    }

    lastChange_ = reader.lastChange();
    if(lastChange_.isNull())
    {
        // should overwrite the file in this program's format with the current last change date
        qDebug("NoteDescriptor::NoteDescriptor : uuid is null, TODO missing implementation of uuid generation");
    }

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

    XmlNoteReader reader(newFilePath);
    reader.read();
    QDateTime laterLastChange = reader.lastChange(); // TODO handle last change is null

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
        // TODO save
        document_->setModified(false);
        return;
    }
}

void NoteDescriptor::setStateChangeEnabled()
{
    stateChangeEnabled = true;
}
