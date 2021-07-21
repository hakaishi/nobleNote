/* nobleNote, a note taking application
 * Copyright (C) 2020 Christian Metscher <hakaishi@web.de>,
                      Fabian Deuchler <Taiko000@gmail.com>

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.

 * nobleNote is licensed under the MIT, see `http://copyfree.org/licenses/mit/license.txt'.
 */

#include "xmlnotereader.h"
#include <QTextCursor>
#include <QDirIterator>
#include <QApplication>


XmlNoteReader::XmlNoteReader(const QString& filePath, QTextDocument *doc)
{
    document_ = doc;
    filePath_ = filePath;
}

void XmlNoteReader::read()
{
    QFile file(filePath_);
    if(!file.open(QIODevice::ReadOnly))
    {
        //qDebug(qPrintable(QString("XmlNoteReader::XmlNoteReader failed : could not open filepath ") + QDir::toNativeSeparators(filePath)));
           return;
    }
    QXmlStreamReader::setDevice(&file);
    parseXml();
    // if uuid wasn't inside the xml text try to get it out of the filename
    if(uuid_.isNull())
        uuid_ = QUuid(QFileInfo(filePath_).baseName());

    // remove the title inside the document and the empty 2nd line, because the title is already in the window title
    QTextCursor cursor(document_); // point to start of the document
    cursor.select(QTextCursor::LineUnderCursor);
    QString titleLine = cursor.selectedText();
    if(titleLine.trimmed() == title_.trimmed())
        cursor.deleteChar();
    cursor.movePosition(QTextCursor::NextBlock,QTextCursor::KeepAnchor,2);
    QString emptyLine = cursor.selectedText();
    if(emptyLine.trimmed().isEmpty())
        cursor.deleteChar();

    file.close(); // local object gets destroyed
}

void XmlNoteReader::parseXml()
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

        if(name() == QLatin1String("note-content"))
        {
            if(!document_) // if no document_ set where note content can be written into
                skipCurrentElement();
            else
                readContent();
        }
        else if(name() == QLatin1String("id") || name() == QLatin1String("uuid")) // only "id" is written
        {
            QString idStr = readElementText();
            uuid_ = parseUuid(idStr);
        }
        else if(name() == QLatin1String("title"))
        {
            title_ = readElementText();
        }
        else if(name() == QLatin1String("last-change-date"))
        {
            lastChange_ = QDateTime::fromString(readElementText(),Qt::ISODate);
        }
        else if(name() == QLatin1String("last-metadata-change"))
        {
            lastMetadataChange_ = QDateTime::fromString(readElementText(),Qt::ISODate);
        }
        else if(name() == QLatin1String("create-date"))
        {
            createDate_ = QDateTime::fromString(readElementText(),Qt::ISODate);
        }
        else if(name() == QLatin1String("tag"))
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
    bool linkFormat = false;
    Q_UNUSED(linkFormat);
    while (!atEnd())
    {
        TokenType token = readNext();

        switch(token)
        {
        // read the text between the formatting elements
        case Characters:
        {
            // this commented out code does not work with formatted text and multiple links
            // TODO a QTextDocumentFragment must be created via QTextCursor::selected, the fragment must be exported to html
            // the link applied and reinserted via QTextCursor::insertHtml
//            if(linkFormat)
//            {
//                qDebug("link inserted");
//                cursor.insertHtml("<a href=\"" + text().toString() + "\">" + text().toString() + "</a>");
//            }
//            else
                cursor.insertText(text().toString(),format);
            break;
        }

            // read elements <bold> <italic> and set the formatting
        case StartElement:
        {
            if(name().toLatin1() == "bold")
                format.setFontWeight(QFont::Bold);

            if(name().toLatin1() == "italic")
                format.setFontItalic(true);

            if(name().toLatin1() == "underline")
                format.setUnderlineStyle(QTextCharFormat::SingleUnderline);

            if(name().toLatin1() == "strikethrough" || name().toLatin1() == "strikeout") // only strikethrough is written, but strikeout is also allowed for reading
                format.setFontStrikeOut(true);

            if(name().toLatin1() == "highlight")
                format.setBackground(QColor(255,255,0));

            if(qualifiedName().toLatin1() == "size:small")
                format.setFontPointSize(QApplication::font().pointSize()* 0.8f);

            if(qualifiedName().toLatin1() == "size:large")
                format.setFontPointSize(QApplication::font().pointSize()*1.4f);

            if(qualifiedName().toLatin1() == "size:huge")
                format.setFontPointSize(QApplication::font().pointSize()*1.6f);

            if(name().toLatin1() == "monospace")
                format.setFontFamilies(QStringList("Monospace"));

            if(qualifiedName().toLatin1() == "link:url")
            {
                linkFormat = true;
            }

                break;
        }
           // unset formatting
        case EndElement:
        {
            if(name().toLatin1() == "note-content") // end of note content, exit this method
                return;

            if(name().toLatin1() == "bold")
                format.setFontWeight(QFont::Normal);

            if(name().toLatin1() == "italic")
                format.setFontItalic(false);

            if(name().toLatin1() == "underline")
                format.setUnderlineStyle(QTextCharFormat::NoUnderline);

            if(name().toLatin1() == "strikethrough" || name().toLatin1() == "strikeout") // only strikethrough is written, but strikeout is also allowed for reading
                format.setFontStrikeOut(false);

            if(name().toLatin1() == "highlight")
                format.clearBackground();

            if(qualifiedName().toLatin1() == "size:small" || qualifiedName().toLatin1() == "size:large" || qualifiedName().toLatin1() == "size:huge") // mutual exclusive options
                format.setFontPointSize(QApplication::font().pointSizeF());

            if(name().toLatin1() == "monospace")
                format.setFontFamilies(QStringList(QApplication::font().family()));

            if(name().toLatin1() == "link:url")
                linkFormat = false;

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
    uuid = QUuid(idStr.right(32 + 4));

    if(uuid.isNull()) // if parsing fails, try more complex parsing
    {
        if(idStr.left(QString("urn:uuid:").length()) == "urn:uuid:") // check if first 9 chars match "urn:uuid:"
        {
            QString uuidRef = idStr.mid(QString("urn:uuid:").length(),32+4); //32 digits and four hyphens
            uuid = QUuid(uuidRef);
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
        if(reader.readNextStartElement() && (reader.name().toLatin1() == "id" || reader.name().toLatin1() == "uuid"))
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
    // if uuid wasn't inside the xml text try to get it out of the filename
    return QUuid(QFileInfo(filePath).baseName());
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
            if(reader.name().toLatin1() == "note")
                return true;
            else if(reader.name().toLatin1() == "html" || reader.name().toLatin1() == "head") // detect html
                return false;
        }
        if(reader.hasError()) // this should detect plain text
            return false;
    }
    return false;
}
