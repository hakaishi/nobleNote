/* nobleNote, a note taking application
 * Copyright (C) 2015 Christian Metscher <hakaishi@web.de>,
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

#ifndef HTMLNOTEREADER_H
#define HTMLNOTEREADER_H

#include "abstractnotereader.h"
#include <QTextFrame>
#include <QDateTime>
#include <QUuid>
#include <QFile>
#include <QTextDocument>

/**
  * a class reading formatted html text and plain text
  * reading requires a QTextFrame
  *
  */

class HtmlNoteReader : public AbstractNoteReader
{
public:
     HtmlNoteReader(const QString &filePath);

     void read();

    // get the uuid that has been extracted during read()
    QUuid uuid() const                         { return uuid_;}


    const QString& title() const            { return title_;} // file name


     // get last change date
    const QDateTime& lastChange() const         { return lastChange_;}

    // get last metadata change date
    const QDateTime& lastMetadataChange() const { return lastMetadataChange_;}

    // get create date
    const QDateTime& createDate() const         { return createDate_;}

    const QString& html() const { return html_;} // returns the html contents of the file

    // returns the title from the html header
    static QString titleFromHtml(const QString& filePath);

//     // reads a uuid from a file, if uuid could not be found, a null uuid is returned
    static QUuid uuid(QString filePath);

//    // searches all directorys under the given path recursively for a file with the given UUID
//    // returns the first file that contains the given uuid or an empty string if the uuid could not be found or if the given uuid is null
    static QString findUuid(const QUuid uuid, const QString & path);

    // reads a uuid from the html contents of string
    static QUuid uuidFromHtml(const QString &html);

    // returns the text of the content attribute in a <meta name="foo" content="bar" /> element for the given name
    static QString metaContent(const QString &html, const QString &name);

private:
    void read(const QString &filePath); // read the content's of a QIODevice and write the formatted text into a QTextDocument
    static QUuid parseUuid(QString idStr);
    void readContent(); // read contents of <note-content> tag
    QString title_;
    QString filePath_;
    QUuid uuid_;
    QDateTime lastChange_;
    QDateTime lastMetadataChange_;
    QDateTime createDate_;
    QString html_;
};

#endif // HTMLNOTEREADER_H
