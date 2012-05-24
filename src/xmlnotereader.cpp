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

            // ignore id

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
    while(!atEnd() && !this->readNextStartElement())
    {
        if(name() == "note-content")
          readContent();

        if(name() == "id" || name() == "uuid") // only "id" is written
        {
            QString idStr = readElementText();

            bool ok = false; // parsing success check variable

            // try to parse the rightmost 32 digits and four hyphens
            uuid_ = QUuid(idStr.rightRef(32 + 4).toString());
            ok = uuid_.isNull();

            if(!ok) // if parsing fails, try more complex parsing
            {
                if(idStr.leftRef(QString("urn:uuid:").length()) == "urn:uuid:") // check if first 9 chars match "urn:uuid:"
                {
                    QStringRef uuidRef = idStr.midRef(QString("urn:uuid:").length(),32+4); //32 digits and four hyphens
                    uuid_ = QUuid(uuidRef.toString());
                    ok = uuid_.isNull();
                }
                else // try to parse the whole string
                {
                    uuid_ = QUuid(idStr.simplified());
                    ok = uuid_.isNull();
                }
            }

            if(!ok)
                qDebug("XmlNoteReader::read : error generating UUID, null UUID has been set");


        }
    }

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
