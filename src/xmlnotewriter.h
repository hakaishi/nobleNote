#ifndef XMLNOTEWRITER_H
#define XMLNOTEWRITER_H

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QTextFrame>
#include <QUuid>
#include <QDateTime>
#include <QFile>

/**
  * a class writing formatted text in xml files
  * the format is similar to the xml format used by tomboy/gnote
  * writing requires a QTextFrame
  *
  * Warning: when using a QIODevice for each of the methods XmlNoteReader::read, XmlNoteWriter::write
  * and the static XmlNoteReader::uuid(QIODevice* devce)
  * the device must be closed and opened separately
  * The same QString* in QXmlStreamWriter cannot be reused in QXmlStreamReader.
  *
  *
  * this class should not be used at the moment because the xml output is missing new line special chars
  * this is due to implementation problems of the detection of QChar::ParagraphSeparator between the QTextFragments and
  * QTextBlocks while iterating over a QTextFragment. See write() for details
  *
  */

class XmlNoteWriter : protected QXmlStreamWriter
{
public:
    XmlNoteWriter();
    XmlNoteWriter(const QString &filePath);

    // Warning: The application will crash if device* points to a local stack object
    // that gets destroyed before write() is called
    void setDevice(QIODevice * device)    { QXmlStreamWriter::setDevice(device);}
    QIODevice * device() const            { return QXmlStreamWriter::device();}


    // obtain this via     QTextFrame* frame = textEdit->document()->rootFrame();
    void setFrame(QTextFrame * frame)       {   frame_ = frame;}
    QTextFrame * frame() const              { return frame_;}

    void setNoteTitle(const QString& title)     { title_ = title;}
    const QString& noteTitle() const            { return title_;}

    // set last change date, if not set, the current date is used
    void setLastChange(const QDateTime& dt)     { lastChange_ = dt;}
    const QDateTime& lastChange() const         { return lastChange_;}

    // set last metadata change date, if not set, the current date is used
    void setLastMetadataChange(const QDateTime& dt)     { lastMetadataChange_ = dt;}
    const QDateTime& lastMetadataChange() const         { return lastMetadataChange_;}

    // set create date, if not set, the current date is used
    void setCreateDate(const QDateTime& dt)     { createDate_ = dt;}
    const QDateTime& createDate() const         { return createDate_;}

    void write(); // write the content's of frame to the specified device/outputString

    void setUuid(QUuid uuid) { uuid_ = uuid;}
    QUuid uuid() const {return uuid_;}


    // TODO clear statement?

private:
    QString title_;
    QTextFrame * frame_;
    QUuid uuid_;
    QDateTime lastChange_;
    QDateTime lastMetadataChange_;
    QDateTime createDate_;

    QFile file;
};

#endif // XMLNOTEWRITER_H
