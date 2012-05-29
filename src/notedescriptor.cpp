#include "notedescriptor.h"
#include <QFile>

NoteDescriptor::NoteDescriptor(QString filePath, QTextDocument* document, QObject *parent) :
    QObject(parent)
{
    filePath_ = filePath;
    document_ = document;
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug("NoteDescriptor::NoteDescriptor failed : could not open filepath");
           return;
    }
    reader.setDevice(&file);
    reader.setFrame(document->rootFrame());
    reader.read();

}

void NoteDescriptor::stateChange()
{

}
