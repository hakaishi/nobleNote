#ifndef XMLNOTEREADER_H
#define XMLNOTEREADER_H

#include <QXmlStreamReader>
#include <QTextFrame>
#include <QUuid>

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

class XmlNoteReader : protected QXmlStreamReader
{
public:
    XmlNoteReader();
    void setDevice(QIODevice * device)     { QXmlStreamReader::setDevice(device);}
    QIODevice * device() const             { return QXmlStreamReader::device();}

    // obtain this via     QTextFrame* frame = textEdit->document()->rootFrame();
    void setFrame(QTextFrame * frame)       {   frame_ = frame;}
    QTextFrame * frame() const              { return frame_;}

    const QString& noteTitle() const            { return title_;}

    void read(); // read the content's of a QIODevice and write the formatted text into a QTextFrame

    inline QUuid uuid() const; // get the uuid that has been extracted during read()

     // reads a uuid from a file, if uuid could not be found, a null uuid is returned
    static QUuid uuid(QIODevice * device);

    // searches all directorys under the given path recursively for a file with the given UUID
    // returns the first file that contains the given uuid or an empty string if the uuid could not be found
    static QString findUuid(const QUuid uuid, const QString & path);

private:
    static QUuid parseUuid(QString idStr);
    void readContent(); // read contents of <note-content> tag
    QString title_;
    QTextFrame * frame_;
    QUuid uuid_;
};


QUuid XmlNoteReader::uuid() const
{
    return uuid_;
}

#endif // XMLNOTEREADER_H