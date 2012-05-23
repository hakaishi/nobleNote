#include "xmlnotewriter.h"
#include <QTextDocument>
#include <qtextcursor.h>

XmlNoteWriter::XmlNoteWriter() : outputString_(NULL), frame_(NULL)
{
    title_ = "untitled note";
}

XmlNoteWriter::XmlNoteWriter(QString *outputString) : QXmlStreamWriter(outputString), outputString_(NULL), frame_(NULL)
{
    title_ = "untitled note";
    outputString_ = outputString;
}

void XmlNoteWriter::write()
{
    if((!outputString_ && !this->QXmlStreamWriter::device()) || !frame_)
    {
        qDebug("XmlNoteWriter::write failed: textframe NULL or both outputString and output device are NULL");
        return;
    }
    setAutoFormatting(true);
    writeStartDocument();
    writeEmptyElement("note");
    writeAttribute("version","0.3");
    writeTextElement("title",title_);
    writeEmptyElement("text");
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
     writeEndElement();
     writeEndDocument();
}


