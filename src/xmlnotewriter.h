#ifndef XMLNOTEWRITER_H
#define XMLNOTEWRITER_H

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QTextFrame>
#include <QUuid>

/**
  * a class writing formatted text in xml files
  * the format is similar to the xml format used by tomboy/gnote
  * writing requires a QTextFrame
  *
  */

class XmlNoteWriter : protected QXmlStreamWriter
{
public:
    XmlNoteWriter();
    XmlNoteWriter(QString* outputString);

    void setOutputDevice(QIODevice * device)    { QXmlStreamWriter::setDevice(device);}
    QIODevice * outputDevice() const            { return QXmlStreamWriter::device();}


    // obtain this via     QTextFrame* frame = textEdit->document()->rootFrame();
    void setFrame(QTextFrame * frame)       {   frame_ = frame;}
    QTextFrame * frame() const              { return frame_;}

    void setNoteTitle(const QString& title)     { title_ = title;}
    const QString& noteTitle() const            { return title_;}

    void write(); // write the content's of frame to the specified device/outputString

    void setUuid(QUuid uuid) { uuid_ = uuid;}
    QUuid uuid() const {return uuid_;}


    // TODO clear statement?

private:
    QString title_;
    QString * outputString_;
    QTextFrame * frame_;
    QUuid uuid_;

};

#endif // XMLNOTEWRITER_H
