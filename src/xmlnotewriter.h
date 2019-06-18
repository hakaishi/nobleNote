/* nobleNote, a note taking application
 * Copyright (C) 2019 Christian Metscher <hakaishi@web.de>,
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

#ifndef XMLNOTEWRITER_H
#define XMLNOTEWRITER_H

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QTextFrame>
#include <QUuid>
#include <QDateTime>
#include <QFile>

/**
  * a class writing formatted text in xml files
  * the format is similar to the xml format used by tomboy/gnote
  * writing requires a QTextFrame
  *
  * Warning: when using a QIODevice for each of the methods XmlNoteReader::read, XmlNoteWriter::write
  * and the static XmlNoteReader::uuid(QIODevice* devce)
  * the device must be closed and opened separately
  * The same QString* in QXmlStreamWriter cannot be reused in QXmlStreamReader.
  *
  *
  * this class should not be used at the moment because the xml output is missing new line special chars
  * this is due to implementation problems of the detection of QChar::ParagraphSeparator between the QTextFragments and
  * QTextBlocks while iterating over a QTextFragment. See write() for details
  *
  */

class XmlNoteWriter : protected QXmlStreamWriter
{
public:
    XmlNoteWriter();
    XmlNoteWriter(const QString &filePath);

    // Warning: The application will crash if device* points to a local stack object
    // that gets destroyed before write() is called
    void setDevice(QIODevice * device)    { QXmlStreamWriter::setDevice(device);}
    QIODevice * device() const            { return QXmlStreamWriter::device();}

    // obtain this via     QTextFrame* frame = textEdit->document()->rootFrame();
    void setFrame(QTextFrame * frame)       {   frame_ = frame;}
    QTextFrame * frame() const              { return frame_;}

    // set last change date, if not set, the current date is used
    void setLastChange(const QDateTime& dt)     { lastChange_ = dt;}
    const QDateTime& lastChange() const         { return lastChange_;}

    // set last metadata change date, if not set, the current date is used
    void setLastMetadataChange(const QDateTime& dt)     { lastMetadataChange_ = dt;}
    const QDateTime& lastMetadataChange() const         { return lastMetadataChange_;}

    // set create date, if not set, the current date is used
    void setCreateDate(const QDateTime& dt)     { createDate_ = dt;}
    const QDateTime& createDate() const         { return createDate_;}

    void write(); // write the content's of frame to the specified device/outputString

    void setUuid(QUuid uuid) { uuid_ = uuid;}
    QUuid uuid() const {return uuid_;}


    // TODO clear statement?

private:
    QTextFrame * frame_;
    QUuid uuid_;
    QDateTime lastChange_;
    QDateTime lastMetadataChange_;
    QDateTime createDate_;

    QFile file;
};

#endif // XMLNOTEWRITER_H
