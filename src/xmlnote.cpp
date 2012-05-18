#include "xmlnote.h"

XmlNote::XmlNote() : outputString_(NULL), frame_(NULL)
{
    title_ = "untitled note";
}

XmlNote::XmlNote(QString *outputString) : QXmlStreamWriter(outputString), outputString_(NULL), frame_(NULL)
{
    title_ = "untitled note";
    outputString_ = outputString;
}

void XmlNote::write()
{
    if((!outputString_ && !this->QXmlStreamWriter::device()) || !frame_)
    {
        qDebug("XmlNote::write failed: textframe NULL or both outputString and output device are NULL");
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
                 writeStartElement("strikeout"),++elements;
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

void XmlNote::read()
{
    if(!this->QXmlStreamReader::device() || !frame_)
    {
        qDebug("XmlNote::read failed: textframe NULL or input device NULL");
        return;
    }

}
