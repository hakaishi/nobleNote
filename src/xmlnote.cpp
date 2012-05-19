#include "xmlnote.h"
#include <QTextDocument>
#include <qtextcursor.h>

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

void XmlNote::readContent()
{
    QTextCursor cursor(frame_);
    QTextCharFormat format;
    while (!atEnd())
    {
        TokenType token = readNext();

        switch(token)
        {
        // read the text between the formatting elements
        case Characters:
        {
            cursor.insertText(text().toString(),format);
            break;
        }

            // read elements <bold> <italic> and set the formatting
        case StartElement:
        {
            if(name() == "bold");
                format.setFontWeight(QFont::Bold);

            if(name() == "italic")
                format.setFontItalic(true);

            if(name() == "underline")
                format.setUnderlineStyle(QTextCharFormat::SingleUnderline);

            if(name() == "strikethrough" || name() == "strikeout") // only strikethrough is written, but strikeout is also allowed for reading
                format.setFontStrikeOut(true);

                break;
        }
           // unset formatting
        case EndElement:
        {
            if(name() == "note-content") // end of note content
                break;

            if(name() == "bold");
                format.setFontWeight(QFont::Normal);

            if(name() == "italic")
                format.setFontItalic(false);

            if(name() == "underline")
                format.setUnderlineStyle(QTextCharFormat::NoUnderline);

            if(name() == "strikethrough" || name() == "strikeout") // only strikethrough is written, but strikeout is also allowed for reading
                format.setFontStrikeOut(false);

            break;
        }
        }

        if (QXmlStreamReader::hasError())
        {
            qDebug("XMLNote::read failed: Error reading xml content");
            return;
        }
    }
}

void XmlNote::read()
{
    if(!this->QXmlStreamReader::device() || !frame_)
    {
        qDebug("XmlNote::read failed: textframe NULL or input device NULL");
        return;
    }

    // skip everything until <note-content>
    while(!atEnd() && !(this->readNextStartElement() && this->name() == "note-content"))
    {
    }

    readContent();

    if (QXmlStreamReader::hasError())
    {
        qDebug("XMLNote::read failed: Error reading xml content");
        return;
    }

}
