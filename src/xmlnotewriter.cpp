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

#include "xmlnotewriter.h"
#include "datetime.h"
#include <QTextDocument>
#include <QTextCursor>
#include <QApplication>
#include <QFile>
#include <QDir>

XmlNoteWriter::XmlNoteWriter() :frame_(NULL)
{
}

XmlNoteWriter::XmlNoteWriter(const QString &filePath) : file(filePath)
{
    if(!file.open(QIODevice::WriteOnly))
    {
        //qDebug(qPrintable(QString("XmlNoteWriter::XmlNoteWriter failed : could not open filepath ") + QDir::toNativeSeparators(filePath)));
           return;
    }
    QXmlStreamWriter::setDevice(&file);
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
    writeTextElement("title",frame_->document()->metaInformation(QTextDocument::DocumentTitle));

    writeStartElement("text");
    writeAttribute("xml:space","preserve");
    writeStartElement("note-content");

     // tomboy compatibility
    writeAttribute("version","0.1");


     for(QTextFrame::Iterator it = frame_->begin(); it != frame_->end(); ++it)
     {
//         QTextCursor cursor(it.currentBlock());
//         cursor.movePosition(QTextCursor::PreviousBlock);
//         cursor.movePosition(QTextCursor::EndOfBlock);
//         cursor.movePosition(QTextCursor::NextBlock,QTextCursor::KeepAnchor);
//         writeCharacters(cursor.selectedText().replace(QChar(QChar::ParagraphSeparator),QString('\n')));

//         qDebug() << "selected Text:" << cursor.selectedText();
         for(QTextBlock::Iterator blit = it.currentBlock().begin(); blit != it.currentBlock().end(); ++blit)
         {
             //qDebug("block iteration");
             int elements = 0;
             if(blit.fragment().charFormat().fontItalic())
                 writeStartElement("italic"),++elements;
             if(blit.fragment().charFormat().fontStrikeOut())
                 writeStartElement("strikethrough"),++elements;
             if(blit.fragment().charFormat().fontWeight() > QFont::Normal)
                 writeStartElement("bold"),++elements;
             if(blit.fragment().charFormat().underlineStyle() != QTextCharFormat::NoUnderline)
                 writeStartElement("underline"),++elements;

             writeCharacters(blit.fragment().text().replace(QChar(QChar::ParagraphSeparator),QString('\n')));
             //writeCharacters(QString('\n'));

             for(int i = 0; i<elements; ++i)
                 writeEndElement();
         }

                  QTextCursor cursor(it.currentBlock());
                  cursor.movePosition(QTextCursor::EndOfBlock);
                  cursor.movePosition(QTextCursor::NextBlock,QTextCursor::KeepAnchor);
                  writeCharacters(cursor.selectedText().replace(QChar(QChar::ParagraphSeparator),QString('\n')));

     }
     writeEndElement(); // "note-content"
     writeEndElement(); // "text"

     // DateTime::toISO8601 generates e.g. 2012-05-26T15:11:15.7750000+02:00
     writeTextElement("last-change-date",       DateTime::toISO8601(lastChange_.isNull()            ? QDateTime::currentDateTime() : lastChange_));
     writeTextElement("last-metadata-change",   DateTime::toISO8601(lastMetadataChange_.isNull()    ? QDateTime::currentDateTime() : lastMetadataChange_));
     writeTextElement("create-date",            DateTime::toISO8601(createDate_.isNull()            ? QDateTime::currentDateTime() : createDate_));


     // tomboy compatibility
     writeStartElement("tags");
     writeTextElement("tag","system:notebook:" + qApp->applicationName());
     writeEndElement();

     writeEndElement(); // "note"
     writeEndDocument();
}


