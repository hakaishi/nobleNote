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

#ifndef XMLNOTEREADER_H
#define XMLNOTEREADER_H

#include "abstractnotereader.h"
#include <QXmlStreamReader>
#include <QTextFrame>
#include <QUuid>
#include <QDateTime>
#include <QFile>
#include <QTextDocument>

/**
  * a class reading formatted text in xml files
  * the format is similar to the xml format used by tomboy/gnote
  * reading requires a QTextFrame
  *
  * Warning: when using a QIODevice for each of the methods XmlNoteReader::read, XmlNoteWriter::write
  * and the static XmlNoteReader::uuid(QIODevice* devce)
  * the device must be closed and opened separately
  *
  */

class XmlNoteReader : public AbstractNoteReader, protected QXmlStreamReader
{
public:
    XmlNoteReader(const QString &filePath, QTextDocument* doc);

    void read();

    // Warning: The application will crash if device* points to a local stack object
    // that gets destroyed before read() is called
    void setDevice(QIODevice * device)     { QXmlStreamReader::setDevice(device);}
    QIODevice * device() const             { return QXmlStreamReader::device();}

    // only the root frame is used via QTextFrame* frame = document_->rootFrame();
    void setDocument(QTextDocument * document)       {   document_ = document;}
    QTextDocument * document() const              { return document_;}

    QUuid uuid() const                         { return uuid_;}       // get the uuid that has been extracted during read()

    const QString& title() const            { return title_;}

     // get last change date
    const QDateTime& lastChange() const         { return lastChange_;}

    // get last metadata change date
    const QDateTime& lastMetadataChange() const { return lastMetadataChange_;}

    // get create date
    const QDateTime& createDate() const         { return createDate_;}

    // return the notebook tag, e.g. system:notebook:MyNotebook
    const QString& tag() const { return tag_; }

    // reads a uuid from a file, if uuid could not be found, a null uuid is returned
    static QUuid uuid(QString filePath);

    // searches all directorys under the given path recursively for a file with the given UUID
    // returns the first file that contains the given uuid or an empty string if the uuid could not be found or if the given uuid is null
    static QString findUuid(const QUuid uuid, const QString & path);

    // heuristic that checks for <note ... > xml element
    static bool mightBeXmlNote(const QString & filePath);

private:
    void parseXml(); // read the content's of a QIODevice and write the formatted text into a QTextFrame
    static QUuid parseUuid(QString idStr);
    void readContent(); // read contents of <note-content> tag
    QString title_;
    QTextDocument * document_;
    QString filePath_;
    QUuid uuid_;
    QDateTime lastChange_;
    QDateTime lastMetadataChange_;
    QDateTime createDate_;
    QString tag_;
};



#endif // XMLNOTEREADER_H
