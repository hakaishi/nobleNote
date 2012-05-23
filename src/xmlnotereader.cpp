#include "xmlnotereader.h"
#include <qtextcursor.h>


XmlNoteReader::XmlNoteReader()
{
}

void XmlNoteReader::readContent()
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
        default:; // suppress compiler warnings
        }

        if (QXmlStreamReader::hasError())
        {
            qDebug("XmlNoteReader::read failed: Error reading xml content");
            return;
        }
    }
}

void XmlNoteReader::read()
{
    if(!this->QXmlStreamReader::device() || !frame_)
    {
        qDebug("XmlNoteReader::read failed: textframe NULL or input device NULL");
        return;
    }

    // skip everything until <note-content>
    while(!atEnd() && !(this->readNextStartElement() && this->name() == "note-content"))
    {
    }

    readContent();

    if (QXmlStreamReader::hasError())
    {
        qDebug("XmlNoteReader::read failed: Error reading xml content");
        return;
    }

}

QUuid XmlNoteReader::uuid() const
{
    return uuid_;
}
