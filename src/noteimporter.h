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

#ifndef NOTEIMPORTER_H
#define NOTEIMPORTER_H

#include <QObject>
#include <QPointer>
#include <QFutureWatcher>
#include "htmlnotewriter.h"
#include "progressreceiver.h"
#if QT_VERSION >= 0x050000
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QFileDialog>
#else
#include <QProgressDialog>
#include <QFileDialog>
#endif

class NoteImporter : public QObject
{
    Q_OBJECT
public:
    explicit NoteImporter(QObject *parent = 0);
    
public slots:
    void importDialog();

signals:
    
private slots:
    void importXmlNotes();

private:
    struct Xml2HtmlFunctor
    {
         ProgressReceiver *p;
         QString path;
         void operator()(const QString &file)
         {
              HtmlNoteWriter::writeXml2Html(file,path);
              p->postProgressEvent();
         }
    };

    Xml2HtmlFunctor xml2HtmlFunctor;

    QProgressDialog *dialog;
    ProgressReceiver *progressReceiver;
    QFutureWatcher<void> *futureWatcher;
    QStringList     importFiles;
    QPointer<QFileDialog> fileDialog;
    QWidget * parentWidget;
    
};

#endif // NOTEIMPORTER_H
