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
  * Warning: when using a QIODevice for both XmlNoteReader and XmlNoteWriter
  * the device must be closed and opened again before it can be used in the other
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

    QUuid uuid() const; // TODO uuid reading not implemented yet TODO uuid format

private:
    void readContent(); // read contents of <note-content> tag
    QString title_;
    QTextFrame * frame_;
    QUuid uuid_;
};

#endif // XMLNOTEREADER_H
