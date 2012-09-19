/* nobleNote, a note taking application
 * Copyright (C) 2012 Christian Metscher <hakaishi@web.de>,
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

#ifndef ABSTRACTNOTEREADER_H
#define ABSTRACTNOTEREADER_H

/**
 * @brief The AbstractNoteReader is an interface for note reader classes
 */

class QUuid;
class QDateTime;
class QString;
class QIODevice;

#include <QSize>

class AbstractNoteReader
{
public:
    AbstractNoteReader();

    virtual QUuid uuid() const = 0; // get the uuid that has been extracted during read()

    virtual const QString& title() const                 = 0;

     // get last change date
    virtual const QDateTime& lastChange() const          = 0;

    // get create date
    virtual const QDateTime& createDate() const          = 0;

     // reads a uuid from a file, if uuid could not be found, a null uuid is returned
    //virtual static QUuid uuid(QString filePath) = 0;

    // searches all directorys under the given path recursively for a file with the given UUID
    // returns the first file that contains the given uuid or an empty string if the uuid could not be found or if the given uuid is null
    //virtual static QString findUuid(const QUuid uuid, const QString & path) = 0;
};

#endif // ABSTRACTNOTEREADER_H
