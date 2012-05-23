#ifndef XMLNOTEREADER_H
#define XMLNOTEREADER_H

#include <QXmlStreamReader>
#include <QTextFrame>
#include <QUuid>

class XmlNoteReader : protected QXmlStreamReader
{
public:
    explicit XmlNoteReader();
    void setInputDevice(QIODevice * device)     { QXmlStreamReader::setDevice(device);}
    QIODevice * inputDevice() const             { return QXmlStreamReader::device();}

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
