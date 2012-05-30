#ifndef XMLNOTEREADER_H
#define XMLNOTEREADER_H

#include <QXmlStreamReader>
#include <QTextFrame>
#include <QUuid>
#include <QDateTime>
#include <QFile>
#include "abstractnotereader.h"

/**
  * a class reading formatted text in xml files
  * the format is similar to the xml format used by tomboy/gnote
  * reading requires a QTextFrame
  *
  * Warning: when using a QIODevice for each of the methods XmlNoteReader::read, XmlNoteWriter::write
  * and the static XmlNoteReader::uuid(QIODevice* devce)
  * the device must be closed and opened separately
  *
  */

class XmlNoteReader : public AbstractNoteReader, protected QXmlStreamReader
{
public:
    XmlNoteReader();
    XmlNoteReader(const QString &filePath);

    // Warning: The application will crash if device* points to a local stack object
    // that gets destroyed before read() is called
    void setDevice(QIODevice * device)     { QXmlStreamReader::setDevice(device);}
    QIODevice * device() const             { return QXmlStreamReader::device();}

    // obtain this via     QTextFrame* frame = textEdit->document()->rootFrame();
    void setFrame(QTextFrame * frame)       {   frame_ = frame;}
    QTextFrame * frame() const              { return frame_;}

    void read(); // read the content's of a QIODevice and write the formatted text into a QTextFrame

    QUuid uuid() const                         { return uuid_;}       // get the uuid that has been extracted during read()

    const QString& noteTitle() const            { return title_;}

     // get last change date
    const QDateTime& lastChange() const         { return lastChange_;}

    // get last metadata change date
    const QDateTime& lastMetadataChange() const { return lastMetadataChange_;}

    // get create date
    const QDateTime& createDate() const         { return createDate_;}

     // reads a uuid from a file, if uuid could not be found, a null uuid is returned
    static QUuid uuid(QIODevice * device);

    // searches all directorys under the given path recursively for a file with the given UUID
    // returns the first file that contains the given uuid or an empty string if the uuid could not be found or if the given uuid is null
    static QString findUuid(const QUuid uuid, const QString & path);

private:
    static QUuid parseUuid(QString idStr);
    void readContent(); // read contents of <note-content> tag
    QString title_;
    QTextFrame * frame_;
    QUuid uuid_;
    QDateTime lastChange_;
    QDateTime lastMetadataChange_;
    QDateTime createDate_;

    QFile file;
};

#endif // XMLNOTEREADER_H
