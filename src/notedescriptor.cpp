#include "notedescriptor.h"
#include <QFile>
#include <QTimer>
#include <QSettings>
#include "htmlnotereader.h"
#include "htmlnotewriter.h"
#include <QMessageBox>
#include <QFileInfo>

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
    HtmlNoteReader reader(filePath_);
    laterLastChange = reader.lastChange();
    if(lastChange_ < laterLastChange) // modified elsewhere
    {
        if(document_->isModified() && QMessageBox::warning(noteWidget_,tr("Note modified"),
                                                           tr("The note has been modified by another program. Should the note be saved under a different name?"
                                                              " Else the note will be reloaded."),
                                                           QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            int counter = 0;
            QString origPath = filePath_;
            while(QFile::exists(filePath_))
            {
                ++counter;
                filePath_ = origPath +  QString(" (%1)").arg(counter);
            }
            uuid_ = QUuid::createUuid();
            title_ = QFileInfo(filePath_).baseName();
            createDate_ = QDateTime::currentDateTime();
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
     writer.setUuid(uuid);
     lastChange_ = QDateTime::currentDateTime();
     writer.setLastChange(lastChange_);
     writer.setLastMetadataChange(lastMetadataChange_);
     writer.setCreateDate(createDate_);
     writer.setTitle(title_);
     writer.write();

     if(noteWidget_)
         noteWidget_->setWindowTitle(title_);
}

void NoteDescriptor::load(const QString& filePath)
{
    // TODO title

    HtmlNoteReader reader(filePath,document_);

    uuid_ = reader.uuid().isNull() ? QUuid::createUuid() : reader.uuid();
   // title_ = reader.title();
    title_ = QFileInfo(filePath).baseName();

    if(noteWidget_)
        noteWidget_->setWindowTitle(title_);

    // dates can be null, HtmlNoteWriter will generate non null dates
    lastChange_ = reader.lastChange();
    createDate_ = reader.createDate();
    lastMetadataChange_ = reader.lastMetadataChange();

    document_->setModified(false); // avoid emit of delayedModificationChanged()
}
