#include "xmlnotereader.h"
#include <qtextcursor.h>
#include <QDirIterator>


XmlNoteReader::XmlNoteReader(const QString& filePath, QTextDocument *doc)
{
    document_ = doc;
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug("XmlNoteReader::XmlNoteReader failed : could not open filepath");
           return;
    }
    QXmlStreamReader::setDevice(&file);
    read();
    // if uuid wasn't inside the xml text try to get it out of the filename
    if(uuid_.isNull())
        uuid_ = QUuid(QFileInfo(filePath).baseName());

    file.close(); // local object gets destroyed
}

void XmlNoteReader::read()
{
    if( !this->QXmlStreamReader::device())
    {
        qDebug("XmlNoteReader::read failed: input device NULL");
        return;
    }

    // skip everything until <note-content>
    while(!atEnd())
    {
        readNext();

        if(name() == "note-content")
        {
            if(!document_) // if no document_ set where note content can be written into
                skipCurrentElement();
            else
                readContent();
        }
        else if(name() == "id" || name() == "uuid") // only "id" is written
        {
            QString idStr = readElementText();
            uuid_ = parseUuid(idStr);
        }
        else if(name() == "title")
        {
            title_ = readElementText();
        }
        else if(name() == "last-change-date")
        {
            lastChange_ = QDateTime::fromString(readElementText(),Qt::ISODate);
        }
        else if(name() == "last-metadata-change")
        {
            lastMetadataChange_ = QDateTime::fromString(readElementText(),Qt::ISODate);
        }
        else if(name() == "create-date")
        {
            createDate_ = QDateTime::fromString(readElementText(),Qt::ISODate);
        }
        else if(name() == "tag")
        {
            tag_ = readElementText();
        }

        if (QXmlStreamReader::hasError())
        {
            qDebug("XmlNoteReader::read failed: Error reading xml content");
            return;
        }
    }
}

void XmlNoteReader::readContent()
{
    QTextCursor cursor(document_->rootFrame());
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
            if(name() == "bold")
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
            if(name() == "note-content") // end of note content, exit this method
                return;

            if(name() == "bold")
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

/*static*/ QUuid XmlNoteReader::parseUuid(QString idStr)
{
    QUuid uuid;
     // try to parse the rightmost 32 digits and four hyphens
    uuid = QUuid(idStr.rightRef(32 + 4).toString());

    if(uuid.isNull()) // if parsing fails, try more complex parsing
    {
        if(idStr.leftRef(QString("urn:uuid:").length()) == "urn:uuid:") // check if first 9 chars match "urn:uuid:"
        {
            QStringRef uuidRef = idStr.midRef(QString("urn:uuid:").length(),32+4); //32 digits and four hyphens
            uuid = QUuid(uuidRef.toString());
        }
        else // try to parse the whole string
        {
            uuid = QUuid(idStr.simplified());
        }
    }

    if(uuid.isNull())
        qDebug("XmlNoteReader::parseUuid : error parsing UUID, null UUID has been generated");

    return uuid;
}

/*static*/ QUuid XmlNoteReader::uuid(QString filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        return QUuid();
    }
    QXmlStreamReader reader(&file);
    while(!reader.atEnd())
    {
        if(reader.readNextStartElement() && (reader.name() == "id" || reader.name() == "uuid"))
        {
            QString idStr = reader.readElementText();
            return parseUuid(idStr);
        }
        if (reader.hasError())
        {
            //qDebug("XmlNoteReader::uuid failed: Error reading xml content, returning null UUID");
            return QUuid();
        }
    }
    return QUuid();
}

/*static*/ QString XmlNoteReader::findUuid(const QUuid uuid, const QString &path)
{
    if(uuid.isNull())
        return QString();

    QDirIterator it(path, QDirIterator::Subdirectories);
    while(it.hasNext())
    {
              QString filePath = it.next();
                  if(uuid == XmlNoteReader::uuid(filePath))
                      return filePath;
    }
    return QString();
}

/*static*/ bool XmlNoteReader::mightBeXmlNote(const QString &filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        return false;
    }
    QXmlStreamReader reader(&file);
    while(!reader.atEnd())
    {
        reader.readNext();
        if(reader.isStartElement())
        {
            if(reader.name() == "note")
                return true;
            else if(reader.name() == "html" || reader.name() == "head") // detect html
                return false;
        }
        if(reader.hasError()) // this should detect plain text
            return false;
    }
    return false;
}
