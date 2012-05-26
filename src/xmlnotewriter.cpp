#include "xmlnotewriter.h"
#include <QTextDocument>
#include <qtextcursor.h>
#include <datetime.h>

XmlNoteWriter::XmlNoteWriter() :frame_(NULL)
{
    title_ = "untitled note";
    lastChange_ = QDateTime::currentDateTime();
}

// must write well formed xml 1.0 for QXmlStreamReader compatibility
void XmlNoteWriter::write()
{
    if(!this->QXmlStreamWriter::device() || !frame_)
    {
        qDebug("XmlNoteWriter::write failed: textframe NULL or output device are NULL");
        return;
    }
    setAutoFormatting(true);
    writeStartDocument();
    writeStartElement("note");
    writeAttribute("version","0.3");
    writeNamespace("http://example.com","link");
    if(!uuid_.isNull())
    {
        QString uuidStr = uuid_.toString().remove(0,1); // uuid without the { } braces
        uuidStr.chop(1);
        writeTextElement("id","urn:uuid:" + uuidStr);
    }
    else
    {
        qDebug("XmlNoteWriter::write() : UUID is null, using generated UUID");
        QString uuidStr = QUuid::createUuid().toString().remove(0,1);
        uuidStr.chop(1);
        writeTextElement("id","urn:uuid:" + uuidStr);
    }
    writeTextElement("title",title_);

    writeStartElement("text");
    writeAttribute("xml:space","preserve");
    writeStartElement("note-content");
    writeAttribute("version","0.1");

     for(QTextFrame::Iterator it = frame_->begin(); it != frame_->end(); ++it)
     {
         for(QTextBlock::Iterator blit = it.currentBlock().begin(); blit != it.currentBlock().end(); ++blit)
         {
             int elements = 0;
             if(blit.fragment().charFormat().fontItalic())
                 writeStartElement("italic"),++elements;
             if(blit.fragment().charFormat().fontStrikeOut())
                 writeStartElement("strikethrough"),++elements;
             if(blit.fragment().charFormat().fontWeight() > QFont::Normal)
                 writeStartElement("bold"),++elements;
             if(blit.fragment().charFormat().underlineStyle() != QTextCharFormat::NoUnderline)
                 writeStartElement("underline"),++elements;

             writeCharacters(blit.fragment().text());

             for(int i = 0; i<elements; ++i)
                 writeEndElement();
         }
     }
     writeEndElement(); // "note-content"
     writeEndElement(); // "text"
     writeTextElement("last-change-date",DateTime::toISO8601(lastChange_)); // e.g. 2012-05-26T15:11:15.7750000+02:00
     writeEndElement(); // "note"
     writeEndDocument();
}


