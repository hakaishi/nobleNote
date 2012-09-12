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

#ifndef NOTEDESCRIPTOR_H
#define NOTEDESCRIPTOR_H

#include "xmlnotereader.h"
#include "xmlnotewriter.h"
#include "textdocument.h"
#include <QObject>
#include <QTextDocument>
#include <QTextBrowser>

/**
 * @brief The NoteDescriptor class provides an abstraction layer to the underlying note
 * file on the filesystem
 * and also manages automatic saving and reloading of note files
 *
 */

class NoteDescriptor : public QObject
{
    Q_OBJECT
public:
    explicit NoteDescriptor(QString filePath, QTextBrowser * textBrowser, TextDocument *document, QWidget *noteWidget = 0);
    const QString& filePath() const { return filePath_; } // return the current filePath
    bool readOnly() const { return readOnly_; }
    QUuid uuid() const { return uuid_;}
    
signals:
    void close(); // emitted if the user wants to close the note via a message box
    
public slots:
    void stateChange();

    // show html source code, for debugging purposes
    //void showSource();

private slots:
    void unlockStateChange();

private:

    void save(const QString &filePath, QUuid uuid); // calls write with note and backup
    void write(const QString &filePath, QUuid uuid); // write note file to disc
    void load(const QString& filePath); // load a note file into the document



    QUuid uuid_;
    QWidget * noteWidget_;
    QString filePath_;
    TextDocument * document_;
    QTextBrowser  * textBrowser_;
    QDateTime lastChange_;
    QDateTime createDate_;
    //QDateTime lastMetadataChange_;
    QString title_;
    bool readOnly_;

    // thread locking mechanism for stateChange(), because if a MessageBox opens inside stateChange()
    // stateChange() can still be called from events from the GUI-Thread
    struct Lock
    {
        Lock(){count++;}
        ~Lock(){count = count > 0 ? count-1 : count;}
        static bool isLocked();
        static int count;
    };

    Lock * initialLock;
};



#endif // NOTEDESCRIPTOR_H
